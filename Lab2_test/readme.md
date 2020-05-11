pull.sh：下载各组代码。

create_folder.sh：创建文件夹准备测试。

test_judge.sh：重新make，添加待访问html，判断是否实现了代理服务器与pipeline。

test_basic.sh：测试代码的基本功能，包括get正确/错误请求，post正确/错误请求，other_method请求。

listen_screen.sh：辅助文件，用于清理多余的screen。

test.lua：在 init 方法中将三个 HTTP请求拼接在一起，实现每次发送三个请求，辅助wrk使用，便于测试pipeline功能。

GitHub_URL.txt：各组链接。

file_name.txt：保存测试文档参数，分别是：测试类型，需要执行的命令，以及结果存储路径。





请先安装wrk！！！

运行方式：

./pull.sh

./create_folder.sh

./test_judge.sh

./test_basic.sh