# tinyRPC
C++Program 完成一个tinyRPC项目



### 日志模块开发
首先需要创建项目：
日志模块：
'''
1.日志级别
2.打印到文件，支持日期命名，以及日志的滚动
3.C格式化风控
4.线程安全
'''

LogLevel
'''
Debug
Info
Error
'''

LogEvent
'''
文件号、行号
MsgNo
进程号
Thread id
日期、以及时间。精确到ms
自定义消息
'''

日志格式
'''
[Level][%y-%m-%d %H:%H:%s.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
'''

Logger 日志器
1、提供打印日志的方法
2、提供日志输出的路径