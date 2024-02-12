set(src_dirs
    "."     
    "main"                            
    "prboom"
    "prboom-esp32-compat"
    "prboom-wad-tables")

set(priv_include_dirs
    "."                                 
    "prboom"
    "prboom-esp32-compat"
    "prboom-esp32-compat/include"
    "prboom-wad-tables"
    "prboom-wad-tables/include")

# In order for the cases defined by `TEST_CASE` in "kernel" to be linked into
# the final elf, the component can be registered as WHOLE_ARCHIVE
idf_component_register(SRC_DIRS ${src_dirs}
                       PRIV_INCLUDE_DIRS ${priv_include_dirs}
                       PRIV_REQUIRES driver
                       PRIV_REQUIRES esp_partition
                       PRIV_REQUIRES spi_flash
                       PRIV_REQUIRES fatfs
                       PRIV_REQUIRES esp_timer)
