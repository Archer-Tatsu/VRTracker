调用方式：
test.exe path subpath filename duration

数据文件将保存在path\subpath路径下，filename为要保存的数据文件名（注意不要带扩展名）
duration是持续时间，单位是秒，最多三位小数精确到毫秒，到时间程序会定时自动关闭

例：test.exe C:\temp my video1 13.035

则数据会保存在C:\temp\my\video1.txt中，运行时间是13.035s