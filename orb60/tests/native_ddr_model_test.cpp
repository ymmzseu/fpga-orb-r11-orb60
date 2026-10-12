// Exercise the C++ DDR/ownership model using pin stubs, independently of HLS.
#define main unused_native_frame_main
#include "native_frame.cpp"
#undef main

int main() {
    try {
        Simulation sim(1,27,6073);
        // Paired HP0 masters contend, but must read the same common memory bytes.
        Bytes pattern(128);for(unsigned i=0;i<128;++i) pattern[i]=uint8_t(i*17+3);
        sim.put(0x1ff8,pattern);
        for(unsigned p: {0u,4u}) {
            sim.ports[p].read_ranges={{0x1ff8,0x2078}};
            sim.dut.m_ARADDR[p]=0x1ff8;sim.dut.m_ARLEN[p]=0;
            sim.dut.m_ARSIZE[p]=3;sim.dut.m_ARBURST[p]=1;sim.dut.m_ARID[p]=1;
            sim.dut.m_ARVALID[p]=1;sim.dut.m_RREADY[p]=0;
        }
        bool accepted[2]={};
        for(unsigned t=0;t<100 && (!accepted[0] || !accepted[1]);++t) {
            sim.tick();
            for(unsigned k=0;k<2;++k) if(sim.dut.m_ARVALID[k*4] && sim.dut.m_ARREADY[k*4]) {
                accepted[k]=true;sim.dut.m_ARVALID[k*4]=0;
            }
        }
        require(accepted[0]&&accepted[1],"Model AR queue test");
        for(unsigned t=0;t<100 && (!sim.ports[0].rvalid || !sim.ports[4].rvalid);++t) sim.tick();
        uint64_t expected=0;for(unsigned i=0;i<8;++i) expected|=uint64_t(pattern[i])<<(8*i);
        for(unsigned p: {0u,4u}) require(sim.ports[p].rvalid && sim.ports[p].rdata==expected && sim.ports[p].rlast && sim.ports[p].rid==1,"Model read payload");
        for(unsigned t=0;t<17;++t) {
            sim.tick();require(sim.ports[0].rdata==expected && sim.ports[4].rdata==expected,"Stalled R payload");
        }
        sim.dut.m_RREADY[0]=sim.dut.m_RREADY[4]=1;sim.tick();
        require(sim.idle(0)&&sim.idle(1),"Model read completion");
        // Four-byte transfer in the upper bus lanes with masked write and B delay.
        sim.ports[7].write_ranges={{0x3004,0x3008}};
        sim.dut.m_AWADDR[7]=0x3004;sim.dut.m_AWLEN[7]=0;sim.dut.m_AWSIZE[7]=2;
        sim.dut.m_AWBURST[7]=1;sim.dut.m_AWID[7]=1;sim.dut.m_AWVALID[7]=1;
        for(unsigned t=0;sim.dut.m_AWVALID[7];++t) {
            require(t<100,"Model AW timeout");sim.tick();if(sim.dut.m_AWREADY[7]) sim.dut.m_AWVALID[7]=0;
        }
        sim.dut.m_WDATA[7]=UINT64_C(0xdeadbeef01020304);sim.dut.m_WSTRB[7]=0xf0;
        sim.dut.m_WLAST[7]=1;sim.dut.m_WVALID[7]=1;sim.dut.m_BREADY[7]=0;
        for(unsigned t=0;sim.dut.m_WVALID[7];++t) {
            require(t<100,"Model W timeout");sim.tick();if(sim.dut.m_WREADY[7]) sim.dut.m_WVALID[7]=0;
        }
        require(sim.word(0x3004)==0xdeadbeef && sim.word(0x3000)==0,"Narrow write lane mask");
        for(unsigned t=0;t<40;++t) sim.tick();
        require(!sim.idle(1) && sim.dut.m_BVALID[7] && sim.dut.m_BID[7]==1,"Delayed B outstanding barrier");
        sim.dut.m_BREADY[7]=1;sim.tick();require(sim.idle(1),"Final B response");
        bool rejected=false;
        try { sim.boundary(0x1ff8,1,3,1,0,false); } catch(const std::runtime_error&) { rejected=true; }
        require(rejected,"Cross-4KB burst accepted");
        rejected=false;
        try { sim.boundary(0x1000,0,3,1,0,false); } catch(const std::runtime_error&) { rejected=true; }
        require(rejected,"Out-of-owned-range read accepted");
        require(sim.ports[0].read_bytes==8 && sim.ports[4].read_bytes==8 && sim.ports[7].write_bytes==4,"Model byte counters");
        std::cout<<"NATIVE_DDR_MODEL_PASS"<<std::endl;return 0;
    } catch(const std::exception& error) { std::cerr<<error.what()<<std::endl;return 1; }
}
