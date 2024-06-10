# httpSever
基于Linux的高并发HTTP服务器
### 预备知识
##### 1、HTTP请求报文及响应报文格式
请求报文格式  
![image](https://github.com/169200750/httpSever/assets/85624974/7facdd4d-91e8-4822-8526-ed4b74dd6df2)

响应报文格式  
![image](https://github.com/169200750/httpSever/assets/85624974/78763ddc-470a-4945-8238-cf0652557cf8)  

##### 2、网络编程socket通信（TCP）
###### 服务端：
（1）创建套接字socket()  
（2）绑定端口号和IP地址bind()   
（3）服务端监听端口listen()  
（4）服务器端接收客户端发出的链接请求accept()  
（5）关闭链接close()  
###### 客户端
本项目基于浏览器进行访问，故为B/S架构，不需手写服务端  

#### 3、处理HTTP请求
（1）分析HTTP报文格式，获取每一行的具体信息  
（2）解析出请求方式、url地址  
（3）比对请求方式，本项目只做了GET的请求方式  
（4）响应HTTP资源请求、发送头部信息，主题信息  
（5）错误处理判断（400、404、500、501）  

#### 4、实现并发处理
在不使用多线程处理的情况下，若有客户端与服务器链接，那么服务器必须等待客户端请求完资源后才能处理下一个客户端的请求，使用多线程可使服务器同时处理多个请求，在客户使用看来，就是服务器单独为其服务  
（1）客户端请求资源后，创建一个线程  
（2）客户端资源请求完毕后，终止该线程  

#### 5、项目演示
##### Linux环境下服务端启动
（1）使用gcc编译程序并启动程序  
![image](https://github.com/169200750/httpSever/assets/85624974/700a2e6d-1c26-4bde-ab70-9e6ae322810a)

（2）打开浏览器输入服务器地址访问资源  
![image](https://github.com/169200750/httpSever/assets/85624974/c2cee5b3-3060-4875-b70e-fae988115596)

（3）观察服务端的调试输出   
![image](https://github.com/169200750/httpSever/assets/85624974/c30973a9-6564-4ab9-9c1a-9222b3ac46e0)

（4）尝试访问不存在的资源  
![image](https://github.com/169200750/httpSever/assets/85624974/8cae3ce4-7cfa-4567-906b-4ec443c7538a)

（5）使用telnet调试格式错误以及请求方法不是GET  
###### 请求格式错误（输入一个空行）
![image](https://github.com/169200750/httpSever/assets/85624974/5a85e933-2e2c-4c55-83da-f68ff5913217)
![image](https://github.com/169200750/httpSever/assets/85624974/36cfe519-f84e-4e93-8ebb-42632ae7a91a)

###### 请求方法不是GET，尝试使用POST进行请求
![image](https://github.com/169200750/httpSever/assets/85624974/2bd023a1-ff51-4c63-935a-cc6e2f7aa976)
这里没有返回信息是因为HTTP报文是以两个回车换行结束，所以我们还需要再次按下回车，再按下回车之前可以先查看服务器所反映的信息  
![image](https://github.com/169200750/httpSever/assets/85624974/1e21e4b1-33e7-4712-b8ec-54946ff5eea0)
按下回车之后，客户端收到服务器发送来的数据信息
![image](https://github.com/169200750/httpSever/assets/85624974/946d94e6-c938-43d6-92e1-0eb621aae405)

#### 项目结束
