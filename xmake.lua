add_requires("jsoncpp")

target("YJson")
    set_languages("cxx17")  -- 指定c++版本

    add_packages("jsoncpp")
    
    set_kind("binary")  -- 生成二进制文件

    add_files("test.cpp")  -- 根目录下.cpp

    add_includedirs("Source")  -- 设置include目录

    add_cxflags("-Wall")