typedef struct
{
    const char*  base_path;          
    int          spi_host_id;         
    int          pin_cs;               

    spp_uint32_t max_files;            
    spp_uint32_t allocation_unit_size; 
    spp_bool_t   format_if_mount_failed;
} SPP_Storage_InitCfg;
