搬运百度网页的日历

数据，界面样式均照搬百度的。

如果提示缺少qml，就安装 qtdeclarative5-*dev

说一下我写的所有的qt软件的编译方法：

方法一，安装qt环境，qtcreator，直接用qtcreator打开编译即可。

方法二，安装qt环境，在代码的目录下新建一个文件夹，然后

qmake ..

（记住 qmake后面 一个空格 再两个“..”） 接着

make

即可。
