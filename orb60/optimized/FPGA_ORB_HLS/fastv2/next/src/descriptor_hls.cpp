/* GPL-3.0-or-later: fixed-point R11 backend, with overlapped DDR patch loading. */
#include "descriptor.hpp"
#include "descriptor_rom.hpp"
#include "next_stream.hpp"
#include "next_block_stream.hpp"
#include <cassert>
#include <cstring>

namespace {
typedef uint64_t RawBlock[158]; // 31 rows x5 words + point(2) + status(1).
typedef uint64_t BlurBlock[222]; // 37 rows x6 words; clipped rows never sampled.
struct PointToken {uint64_t xy,score_tile;};
struct FeaturePacket {uint64_t words[8];bool valid;};
struct Completion {uint32_t raw_bytes,blur_bytes,completed,cycles;int status;};

uint64_t fields(uint32_t a,uint32_t b) {
#pragma HLS INLINE
    return uint64_t(a)|(uint64_t(b)<<32);
}
int32_t signed32(uint32_t value) {
#pragma HLS INLINE
    return value<=UINT32_C(0x7fffffff) ? int32_t(value) : int32_t(int64_t(value)-INT64_C(4294967296));
}
bool valid_point(const PointToken& p,int width,int height) {
#pragma HLS INLINE
    const int x=signed32(uint32_t(p.xy)),y=signed32(uint32_t(p.xy>>32));
    return x>=0 && x<width-32 && y>=0 && y<height-32;
}
int64_t shift_floor(int64_t value,int amount) {
#pragma HLS INLINE
    return value>=0 ? int64_t(uint64_t(value)>>amount) : -INT64_C(1)-int64_t(uint64_t(-(value+1))>>amount);
}
uint32_t precise_angle(int32_t m10,int32_t m01) {
#pragma HLS INLINE off
    if(!m10 && !m01) return 0;
    if(!m01) return m10<0 ? 180u*65536u : 0;
    if(!m10) return m01<0 ? 270u*65536u : 90u*65536u;
    int64_t x=int64_t(m10)*(INT64_C(1)<<20),y=int64_t(m01)*(INT64_C(1)<<20),z=0;
    if(x<0) {x=-x;y=-y;z=INT64_C(180)<<24;}
CordicIterations:
    for(int k=0;k<32 && y!=0;++k) {
#pragma HLS PIPELINE II=1
        const int64_t old=x;
        if(y>0) {x+=shift_floor(y,k);y-=shift_floor(old,k);z+=orb60_descriptor_rom::kCordic[k];}
        else {x-=shift_floor(y,k);y+=shift_floor(old,k);z-=orb60_descriptor_rom::kCordic[k];}
    }
    // Sum of all CORDIC angles <100 degrees: z lies in (-100,280).
    const int64_t full=INT64_C(360)<<24;
    if(z<0) z+=full;
    if(z>=full) z-=full;
    uint32_t rounded=uint32_t(uint64_t(z)>>8);
    const uint32_t remainder=uint32_t(z)&255u;
    if(remainder>128u || (remainder==128u && (rounded&1u))) ++rounded;
    if(rounded>=360u*65536u) rounded-=360u*65536u;
    return rounded;
}
int quantize_angle(uint32_t angle) {
#pragma HLS INLINE off
    // Exact threshold search, including the top half-bin wrapping to zero.
    const uint32_t rounded=angle+368640u;
    int lo=0,hi=33;
QuantizeSearch:
    for(int k=0;k<6;++k) {
#pragma HLS PIPELINE II=1
        const int mid=(lo+hi)>>1;
        if(rounded>=uint32_t(mid)*737280u) lo=mid; else hi=mid;
    }
    return lo&31;
}

void load_points(const uint64_t* points,int count,hls::stream<PointToken>& raw,
    hls::stream<PointToken>& blur) {
#pragma HLS INLINE off
LoadDescriptorPoints:
    for(int i=0;i<count;++i) {
        uint64_t words[2];std::memcpy(words,points+2*i,16);
        const PointToken p={words[0],words[1]};raw.write(p);blur.write(p);
    }
}

void load_raw(const uint64_t* image,hls::stream<PointToken>& points,
    hls::stream_of_blocks<RawBlock>& blocks,int count,int width,int height,
    hls::stream<uint32_t>& done) {
#pragma HLS INLINE off
    uint32_t bytes=0;
    for(int i=0;i<count;++i) {
        const PointToken point=points.read();
        hls::write_lock<RawBlock> lock(blocks);RawBlock& block=lock;
        const bool valid=valid_point(point,width,height);
        block[155]=point.xy;block[156]=point.score_tile;block[157]=valid ? 0 : 2;
        if(valid) {
            const int x=signed32(uint32_t(point.xy))+1,y=signed32(uint32_t(point.xy>>32))+1;
LoadRawPatchRows:
            for(int row=0;row<31;++row) {
                const int address=(y+row)*width+x;
                const int words=((address&7)+31+7)>>3;
                std::memcpy(block+row*5,image+(address>>3),words*8);
                bytes+=words*8;
            }
        }
    }
    done.write(bytes);
}
void load_blur(const uint64_t* image,hls::stream<PointToken>& points,
    hls::stream_of_blocks<BlurBlock>& blocks,int count,int width,int height,
    hls::stream<uint32_t>& done) {
#pragma HLS INLINE off
    uint32_t bytes=0;
    for(int i=0;i<count;++i) {
        const PointToken point=points.read();
        hls::write_lock<BlurBlock> lock(blocks);BlurBlock& block=lock;
        if(valid_point(point,width,height)) {
            const int cx=signed32(uint32_t(point.xy))+16,cy=signed32(uint32_t(point.xy>>32))+16;
            const int x=cx<18 ? 0 : cx-18,last=cx+18>=width ? width-1 : cx+18;
LoadBlurPatchRows:
            for(int row=0;row<37;++row) {
                const int y=cy-18+row;
                if(y>=0 && y<height) {
                    const int address=y*width+x,words=((address&7)+(last-x+1)+7)>>3;
                    std::memcpy(block+row*6,image+(address>>3),words*8);
                    bytes+=words*8;
                }
            }
        }
    }
    done.write(bytes);
}
uint8_t raw_pixel(const RawBlock& block,const uint8_t align[31],int row,int col) {
#pragma HLS INLINE
    const int byte=int(align[row])+col;
    return uint8_t(block[row*5+(byte>>3)]>>((byte&7)*8));
}
uint8_t blur_pixel(const BlurBlock& block,const uint8_t align[37],int row,int col) {
#pragma HLS INLINE
    const int byte=int(align[row])+col;
    return uint8_t(block[row*6+(byte>>3)]>>((byte&7)*8));
}
int signed6(uint32_t value) {
#pragma HLS INLINE
    const int v=value&63;return (v&32) ? v-64 : v;
}
void rotate(int& x,int& y,int quarter) {
#pragma HLS INLINE
    const int old=x;
    if(quarter==1) {x=-y;y=old;}
    else if(quarter==2) {x=-x;y=-y;}
    else if(quarter==3) {x=y;y=-old;}
}

bool describe_patch(const RawBlock& raw,const BlurBlock& blur,int width,int height,
    int level,FeaturePacket& packet) {
#pragma HLS INLINE off
    const int cx=signed32(uint32_t(raw[155]))+16,cy=signed32(uint32_t(raw[155]>>32))+16;
    const uint32_t score=uint32_t(raw[156]);
    uint8_t raw_align[31],blur_align[37];
#pragma HLS ARRAY_PARTITION variable=raw_align complete
#pragma HLS ARRAY_PARTITION variable=blur_align complete
RawRowAlignment:
    for(int row=0;row<31;++row) {
#pragma HLS PIPELINE II=1
        raw_align[row]=uint8_t(((cy-15+row)*width+cx-15)&7);
    }
    const int bx=cx<18 ? 0 : cx-18;
BlurRowAlignment:
    for(int row=0;row<37;++row) {
#pragma HLS PIPELINE II=1
        const int y=cy-18+row;
        blur_align[row]=y>=0 && y<height ? uint8_t((y*width+bx)&7) : 0;
    }
    int32_t m10=0,m01=0;
MomentCenterRow:
    for(int u=-15;u<=15;++u) {
#pragma HLS PIPELINE II=1
        m10+=u*int(raw_pixel(raw,raw_align,15,u+15));
    }
    for(int v=1;v<=15;++v) {
        int32_t vertical=0;
        const int extent=orb60_descriptor_rom::kUmax[v];
MomentSymmetricPairs:
        for(int u=-extent;u<=extent;++u) {
#pragma HLS PIPELINE II=1
            const int plus=raw_pixel(raw,raw_align,15+v,u+15),minus=raw_pixel(raw,raw_align,15-v,u+15);
            vertical+=plus-minus;m10+=u*(plus+minus);
        }
        m01+=v*vertical;
    }
    const int bin=quantize_angle(precise_angle(m10,m01));
    uint32_t bits=0,descriptor[8];
#pragma HLS ARRAY_PARTITION variable=descriptor complete
    bool valid=true;
BriefBits:
    for(int bit=0;bit<256;++bit) {
#pragma HLS PIPELINE II=1
        const uint32_t offsets=uint32_t(orb60_descriptor_rom::kBriefPairs[(bin&7)*256+bit]);
        int ax=signed6(offsets),ay=signed6(offsets>>6),dx=signed6(offsets>>12),dy=signed6(offsets>>18);
        rotate(ax,ay,bin>>3);rotate(dx,dy,bin>>3);
        const bool inside=cx+ax>=0 && cx+ax<width && cy+ay>=0 && cy+ay<height &&
            cx+dx>=0 && cx+dx<width && cy+dy>=0 && cy+dy<height;
        bool less=false;
        if(inside) less=blur_pixel(blur,blur_align,ay+18,cx+ax-bx)<blur_pixel(blur,blur_align,dy+18,cx+dx-bx);
        else valid=false;
        if(less) bits|=UINT32_C(1)<<(bit&31);
        if((bit&31)==31) {descriptor[bit>>5]=bits;bits=0;}
    }
    packet.words[0]=fields(uint32_t(cx),uint32_t(cy));packet.words[1]=fields(score,uint32_t(level));
    packet.words[2]=fields(uint32_t(bin),uint32_t(m10));packet.words[3]=fields(uint32_t(m01),uint32_t(bin)*737280u);
    for(int k=0;k<4;++k) {
#pragma HLS UNROLL
        packet.words[k+4]=fields(descriptor[2*k],descriptor[2*k+1]);
    }
    return valid;
}
void compute_descriptors(hls::stream_of_blocks<RawBlock>& raw,
    hls::stream_of_blocks<BlurBlock>& blur,int count,int width,int height,int level,
    hls::stream<FeaturePacket>& records,hls::stream<int>& done) {
#pragma HLS INLINE off
    int status=0;
    for(int i=0;i<count;++i) {
        hls::read_lock<RawBlock> raw_lock(raw);RawBlock& raw_patch=raw_lock;
        hls::read_lock<BlurBlock> blur_lock(blur);BlurBlock& blur_patch=blur_lock;
        FeaturePacket packet={};
        if(!status) {
            if(raw_patch[157] || !describe_patch(raw_patch,blur_patch,width,height,level,packet)) status=-2;
            else packet.valid=true;
        }
        records.write(packet); // Invalid points still drain every ownership token.
    }
    done.write(status);
}
void write_features(hls::stream<FeaturePacket>& input,uint64_t* features,
    int count,int offset,hls::stream<uint32_t>& done) {
#pragma HLS INLINE off
    uint32_t completed=0;
WriteDescriptorRecords:
    for(int i=0;i<count;++i) {
        const FeaturePacket packet=input.read();
        if(packet.valid) {
            std::memcpy(features+(offset+completed)*8,packet.words,64);++completed;
        }
    }
    done.write(completed);
}
void count_cycles(hls::stream<uint32_t>& raw_done,hls::stream<uint32_t>& blur_done,
    hls::stream<uint32_t>& write_done,hls::stream<int>& compute_done,hls::stream<Completion>& result) {
#pragma HLS INLINE off
    bool raw=false,blur=false,write=false,compute=false;
    Completion c={};
CountDescriptorCycles:
    for(;;) {
#pragma HLS PIPELINE II=1
        if(!raw && raw_done.read_nb(c.raw_bytes)) raw=true;
        if(!blur && blur_done.read_nb(c.blur_bytes)) blur=true;
        if(!write && write_done.read_nb(c.completed)) write=true;
        if(!compute && compute_done.read_nb(c.status)) compute=true;
        ++c.cycles;
        if(raw && blur && write && compute) break;
    }
    result.write(c);
}
void descriptor_dataflow(const uint64_t* raw,const uint64_t* blur,const uint64_t* points,
    uint64_t* features,int width,int height,int count,int level,int offset,hls::stream<Completion>& result) {
#pragma HLS INLINE off
    hls::stream<PointToken> raw_points,blur_points;
    hls::stream_of_blocks<RawBlock> raw_blocks;
    hls::stream_of_blocks<BlurBlock> blur_blocks;
    hls::stream<FeaturePacket> records;
    hls::stream<uint32_t> raw_done,blur_done,write_done;
    hls::stream<int> compute_done;
#pragma HLS STREAM variable=raw_points depth=4
#pragma HLS STREAM variable=blur_points depth=4
#pragma HLS STREAM variable=records depth=2
#pragma HLS STREAM variable=raw_done depth=2
#pragma HLS STREAM variable=blur_done depth=2
#pragma HLS STREAM variable=write_done depth=2
#pragma HLS STREAM variable=compute_done depth=2
#pragma HLS AGGREGATE variable=raw_points compact=bit
#pragma HLS AGGREGATE variable=blur_points compact=bit
#pragma HLS AGGREGATE variable=records compact=bit
#pragma HLS BIND_STORAGE variable=raw_blocks type=ram_t2p impl=bram
#pragma HLS BIND_STORAGE variable=blur_blocks type=ram_t2p impl=bram
#pragma HLS DATAFLOW
    load_points(points,count,raw_points,blur_points);
    load_raw(raw,raw_points,raw_blocks,count,width,height,raw_done);
    load_blur(blur,blur_points,blur_blocks,count,width,height,blur_done);
    compute_descriptors(raw_blocks,blur_blocks,count,width,height,level,records,compute_done);
    write_features(records,features,count,offset,write_done);
    count_cycles(raw_done,blur_done,write_done,compute_done,result);
#ifndef __SYNTHESIS__
    assert(raw_points.empty() && blur_points.empty() && raw_blocks.empty() && blur_blocks.empty() && records.empty());
    assert(raw_done.empty() && blur_done.empty() && write_done.empty() && compute_done.empty());
#endif
}
void write_stats(uint64_t* output,const orb60::DescriptorStats& s) {
#pragma HLS INLINE
    uint64_t words[8];
    words[0]=fields(s.version,s.status);words[1]=fields(s.level,s.requested);
    words[2]=fields(s.completed,s.frame_offset);words[3]=fields(s.raw_read_bytes,s.blur_read_bytes);
    words[4]=fields(s.point_read_bytes,s.feature_write_bytes);words[5]=fields(s.dataflow_cycles,s.flags);
    words[6]=0;words[7]=0;std::memcpy(output,words,64);
}
}

extern "C" int fastv2_descriptor_hls(const uint64_t* raw,const uint64_t* blurred,
    const uint64_t* points,uint64_t* features,uint64_t* stats,int width,int height,
    int count,int level,int frame_offset,int capacity) {
#pragma HLS INTERFACE m_axi port=raw offset=slave bundle=gmem0 depth=86400 max_read_burst_length=16 num_read_outstanding=2 max_widen_bitwidth=64
#pragma HLS INTERFACE m_axi port=blurred offset=slave bundle=gmem1 depth=86400 max_read_burst_length=16 num_read_outstanding=2 max_widen_bitwidth=64
#pragma HLS INTERFACE m_axi port=points offset=slave bundle=gmem2 depth=512 max_read_burst_length=16 num_read_outstanding=2 max_widen_bitwidth=64
#pragma HLS INTERFACE m_axi port=features offset=slave bundle=gmem3 depth=8000 max_write_burst_length=16 num_write_outstanding=2 max_widen_bitwidth=64
#pragma HLS INTERFACE m_axi port=stats offset=slave bundle=gmem3 depth=8 max_write_burst_length=16 num_write_outstanding=2 max_widen_bitwidth=64
#pragma HLS INTERFACE s_axilite port=raw bundle=control
#pragma HLS INTERFACE s_axilite port=blurred bundle=control
#pragma HLS INTERFACE s_axilite port=points bundle=control
#pragma HLS INTERFACE s_axilite port=features bundle=control
#pragma HLS INTERFACE s_axilite port=stats bundle=control
#pragma HLS INTERFACE s_axilite port=width bundle=control
#pragma HLS INTERFACE s_axilite port=height bundle=control
#pragma HLS INTERFACE s_axilite port=count bundle=control
#pragma HLS INTERFACE s_axilite port=level bundle=control
#pragma HLS INTERFACE s_axilite port=frame_offset bundle=control
#pragma HLS INTERFACE s_axilite port=capacity bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control
    orb60::DescriptorStats s={};s.version=orb60::kDescriptorVersion;
    s.level=uint32_t(level);s.requested=uint32_t(count);s.frame_offset=uint32_t(frame_offset);
    if(width<33 || width>960 || height<33 || height>720 || count<0 || count>256 ||
        level<0 || level>7 || frame_offset<0 || capacity<0 || capacity>1000 ||
        frame_offset>capacity || count>capacity-frame_offset) {
        s.status=uint32_t(-1);write_stats(stats,s);return -1;
    }
    hls::stream<Completion> completion;
#pragma HLS STREAM variable=completion depth=2
    descriptor_dataflow(raw,blurred,points,features,width,height,count,level,frame_offset,completion);
    const Completion c=completion.read();
    s.status=uint32_t(c.status);s.completed=c.completed;s.raw_read_bytes=c.raw_bytes;s.blur_read_bytes=c.blur_bytes;
    s.point_read_bytes=uint32_t(count)*16;s.feature_write_bytes=c.completed*64;s.dataflow_cycles=c.cycles;
    s.flags=3;write_stats(stats,s);return c.status;
}
#if !defined(__SYNTHESIS__) && !defined(FASTV2_HLS_CSIM)
uint32_t orb60_descriptor_angle_q16(int32_t m10,int32_t m01) {return precise_angle(m10,m01);}
int orb60_descriptor_quantize(uint32_t angle) {return quantize_angle(angle);}
#endif
