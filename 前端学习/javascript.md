# 引入方式

## 内联引入
```html
<script>
    // JavaScript 代码
</script>
```

## 外部引入
```html
<script src="script.js"></script>
```

# JavaScript 基础语法
## 变量声明
有三种声明方式：`var`（不常用）、`let`、`const`
var 声明的变量可以在函数作用域内访问，let 和 const 声明的变量只能在块级作用域内访问。
其中，`const` 声明的变量是常量，不能被重新赋值，要在声明时初始化。

```javascript
let name = "张三";
let age = 25;
const isStudent = true;
```

## 数据类型
JavaScript 中有七种基本数据类型：`Number`、`String`、`Boolean`、`Null`、`Undefined`、`Symbol` 和 `BigInt`，以及一种复杂数据类型：`Object`。

```javascript
let num = 10;
let str = "Hello, World!";
let bool = true;
let nullVal = null;
let undefinedVal = undefined;
let symbolVal = Symbol("description");//表示唯一的值，用于对象属性的唯一标识符
let bigIntVal = 123456789012345678901234567890n;//表示大整数，超过 Number 类型的最大值
let obj = { name: "张三", age: 25 };//表示对象，包含键值对
```
# 输出
## 控制台输出
```javascript
console.log(name);
console.log(age);
console.log(isStudent);
```
括号里的内容可以是任意类型的值，输出到浏览器的控制台中。
## 弹窗输出
```javascript   
alert(name);
alert(age);
alert(isStudent);
```
括号里的内容可以是任意类型的值，输出到浏览器的弹窗中。

# 运算符
## 算术运算符
## 赋值运算符
## 比较运算符
===：严格相等，比较两个值是否相等且类型相同。
！==：严格不相等，比较两个值是否不相等或类型不同。

# 条件判断
## if 语句
```javascript
if (age >= 18) {
    console.log("已成年");
} else {
    console.log("未成年");
}
```
语法结构：
```javascript   
if (条件) {
    // 条件为 true 时执行的代码
} else {
    // 条件为 false 时执行的代码
}
```
## switch 语句
```javascript
let fruit = "苹果";
switch (fruit) {
    case "苹果":
        console.log("这是苹果");
        break;
    case "香蕉":
        console.log("这是香蕉");
        break;
    default:
        console.log("未知水果");
}
```
# 循环语句
## for 循环
```javascript
for (let i = 0; i < 5; i++) {
    console.log(i);
}
```
## while 循环
```javascript
let i = 0;
while (i < 5) {
    console.log(i);
    i++;
}
```

# 函数
## 函数声明
```javascript
function greet(name) {
    console.log("Hello, " + name + "!");
}
```
## 箭头函数（现代写法）
```javascript
const greet = (name) => {
    console.log("Hello, " + name + "!");
};
```

# 数组
## 创建数组
```javascript
let fruits = ["苹果", "香蕉", "橙子"];
```
## 获取元素
```javascript
console.log(fruits[0]); // 输出 "苹果"
``` 
## 数组方法
### push()：向数组末尾添加一个或多个元素。
```javascript
fruits.push("葡萄");
console.log(fruits); // 输出 ["苹果", "香蕉", "橙子", "葡萄"]
```
### pop()：删除最后一个元素
```javascript
fruits.pop();
console.log(fruits); // 输出 ["苹果", "香蕉", "橙子"]
```
### shift()：删除第一个元素
```javascript
fruits.shift();
console.log(fruits); // 输出 ["香蕉", "橙子"]
```
### unshift()：向数组开头添加一个或多个元素。
```javascript
fruits.unshift("草莓");
console.log(fruits); // 输出 ["草莓", "苹果", "香蕉", "橙子"]
```
### splice()：删除或替换数组中的元素。
```javascript
fruits.splice(1, 1); // 删除索引为1的元素
console.log(fruits); // 输出 ["苹果", "橙子"]
```
### slice()：返回数组的一个片段。
```javascript   
let newFruits = fruits.slice(0, 2); // 获取索引0到1的元素
console.log(newFruits); // 输出 ["苹果", "香蕉"]
```
# 对象
用于存储键值对数据，键是字符串，值可以是任意类型。
```javascript
let person = {
    name: "张三",
    age: 25,
    isStudent: true
};
```
访问对象属性：
```javascript
console.log(person.name); // 输出 "张三"
console.log(person.age); // 输出 25
console.log(person.isStudent); // 输出 true
```
# DOM操作
js可以通过 DOM（文档对象模型）来操作 HTML 元素，实现动态效果。
## 获取元素（id、class、tag）
```javascript
let element = document.getElementById("myElement");
```
# 事件处理
网页交互核心
```javascript
element.addEventListener("click", function() {
    console.log("元素被点击了");
});
```

# 字符串
## 常用方法
### length：获取字符串长度
```javascript
let str = "Hello, World!";
console.log(str.length); // 输出 13
```
### toUpperCase()：将字符串转换为大写
```javascript
let str = "Hello, World!";
console.log(str.toUpperCase()); // 输出 "HELLO, WORLD!"
```
### toLowerCase()：将字符串转换为小写
```javascript
let str = "Hello, World!";
console.log(str.toLowerCase()); // 输出 "hello, world!"
```
### indexOf()：返回指定子字符串在字符串中首次出现的位置，如果未找到则返回 -1
```javascript
let str = "Hello, World!";
console.log(str.indexOf("World")); // 输出 7
console.log(str.indexOf("JavaScript")); // 输出 -1
```
## 模板字符串
使用反引号（``）包裹字符串，可以在字符串中嵌入变量和表达式，使用 `${}` 语法。
```javascript
let name = "张三";
let age = 25;
let str = `我的名字是 ${name}，今年 ${age} 岁。`;
console.log(str); // 输出 "我的名字是 张三，今年 25 岁。"
```
为什么使用模板字符串？
1. **可读性更高**：模板字符串允许在字符串中直接嵌入变量和表达式，使代码更易读。
2. **支持多行字符串**：使用模板字符串可以轻松创建多行字符串，而不需要使用换行符或字符串拼接。

# JSON
JSON（JavaScript Object Notation）是一种轻量级的数据交换格式，易于人阅读和编写，同时也易于机器解析和生成。JSON 是一种文本格式，完全独立于编程语言，但它使用了类似于 JavaScript 的语法。
## JSON 格式
JSON 数据由键值对组成，键是字符串，值可以是字符串、数字、布尔值、数组或对象。
```json
{
    "name": "张三",
    "age": 25,
    "isStudent": true
}
```
# 异步编程
## 回调函数
定义：回调函数是作为参数传递给另一个函数的函数，当某个操作完成后，回调函数会被调用。
```javascript
function fetchData(callback) {
    // 模拟异步操作
    setTimeout(() => {
        let data = "获取到的数据";
        callback(data);
    }, 1000);
}

fetchData(function(data) {
    console.log(data);
});
```
## Promise
定义：Promise 是一种用于处理异步操作的对象，它代表一个可能还未完成的操作，并允许你在操作完成后执行相应的回调函数。
```javascript
function fetchData() {
    return new Promise((resolve, reject) => {
        // 模拟异步操作
        setTimeout(() => {
            let data = "获取到的数据";
            resolve(data);
        }, 1000);
    });
}

fetchData().then((data) => {
    console.log(data);
});
```
## setTimeout 和 setInterval
### setTimeout
定义：setTimeout 用于在指定的时间后执行一个函数，只执行一次。
```javascript
setTimeout(() => {
    console.log("Hello, World!");
}, 1000);
```
### setInterval
定义：setInterval 用于每隔指定的时间执行一个函数，直到调用 clearInterval 停止。
```javascript   
let intervalId = setInterval(() => {
    console.log("Hello, World!");
}, 1000);
```
