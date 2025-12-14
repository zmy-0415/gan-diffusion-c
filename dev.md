# 1. 清空旧构建缓存（关键）
rm -rf build && mkdir build && cd build

# 2. 生成 Makefile（无循环依赖报错）
cmake ..

# 3. 编译（自动拷贝 assets 到 build 目录）
make -j4

# 4. 若修改了 assets 目录文件，手动同步（无需重新编译代码）
make sync_assets

# 5. 运行程序（可访问 build/assets 目录）
./main.exe