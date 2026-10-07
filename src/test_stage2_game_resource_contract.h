#pragma once

// 只读取已完成资源初始化的真实状态，产物由外部验证器对照原光盘资源。
template<class Session>
void DumpStage2GameResources(Session& s,const std::filesystem::path& output){
    const auto dump=[&](const std::string& name,uint32_t address,uint32_t bytes){
        std::ofstream file(output/std::filesystem::u8path(name),std::ios::binary);
        for(uint32_t i=0;i<bytes;++i){const char value=char(s.Read8(address+i));file.write(&value,1);}
        if(!file)throw std::runtime_error("Game resource evidence write failed");
    };
    std::cout<<"resource-state portrait="<<s.Read32(0x8006ED5Cu)
        <<" camera-count="<<s.Read32(0x8006EA40u)<<" camera-flags="<<s.Read32(0x8006EA48u)
        <<" vram-uploads="<<s.Vram().UploadCount()<<" heap-count="<<s.Read32(0x8006EB84u)
        <<" main-frame="<<s.Read32(0x8006EA54u)<<" main-limit="<<s.Read32(0x8006EA58u)
        <<" projection="<<static_cast<PrStage2LifecycleDirect::Services&>(s).MatrixGte().h<<'\n';
    dump("game_camera.bin",0x80092880u,32u);dump("game_camera_copy.bin",0x800901D8u,32u);
    dump("game_morph_weights.bin",0x80090240u,5120u);
    for(uint32_t channel=0;channel<2u;++channel){
        const uint32_t model=0x80074B08u+12u*channel,morph=0x80074B80u+12u*channel;
        const uint32_t count=s.Read32(model),morphCount=s.Read32(morph);
        if(count>4u||morphCount>128u)throw std::runtime_error("Game resource output bounds changed");
        uint32_t backup=s.Read32(model+4u);
        std::cout<<"resource-channel "<<channel<<' '<<count<<' '<<morphCount<<'\n';
        const auto prefix="game_channel_"+std::to_string(channel);
        dump(prefix+"_models.bin",s.Read32(model+8u),28u*count);
        dump(prefix+"_morph.bin",s.Read32(morph+4u),16u*morphCount);
        const uint32_t tracks=channel?0x801D6E9Cu:0x80078BF8u;
        dump(prefix+"_tracks.bin",tracks,16u);
        for(uint32_t i=0;i<count;++i){
            const uint32_t record=s.Read32(model+8u)+28u*i;
            const uint32_t vertices=s.Read32(record),vertexCount=s.Read32(record+4u);
            if(vertexCount>2048u)throw std::runtime_error("Game vertex output bounds changed");
            const auto name=prefix+"_object_"+std::to_string(i);
            dump(name+"_vertices.bin",vertices,8u*vertexCount);
            dump(name+"_backup.bin",backup,8u*vertexCount);
            std::cout<<"resource-model "<<channel<<' '<<i<<' '<<vertices<<' '<<vertexCount<<' '<<backup<<'\n';
            backup+=8u*vertexCount;
        }
    }
}
