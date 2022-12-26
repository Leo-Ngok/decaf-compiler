## **编译原理**

### MiniDecaf 编译器实验报告 -- STAGE 4

2021010706 岳章乔

#### 一、 思考题

##### step 9:

1.

```c
int foo(int x, int y) {
	return x + y;
}
int main() {
    int x = 3;
    int y = 2;
    return foo(x = y, y = x);
}
```

这样其返回值既有可能是 4， 也有可能是 6 。至于实际上是哪种情况，取决于编译器的实现。

2. 

如果只有被调用者保存的寄存器