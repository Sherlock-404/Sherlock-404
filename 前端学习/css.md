# 简介
CSS (Cascading Style Sheets) 是一种用于控制网页外观和格式的样式表语言。它允许开发者定义 HTML 元素的视觉表现，如颜色、字体、间距等。
使用示例：
```css
body {
    background-color: #f0f0f0;
    font-family: Arial, sans-serif;
}
h1 {
    color: #333333;
    text-align: center;
}
```
css以一个选择器开头，选择器可以是HTML元素、类、ID等，后面跟着一对大括号 `{}`，大括号内包含一组属性和对应的值，每个属性和值之间用冒号 `:` 分隔，属性之间用分号 `;` 分隔。

# 浏览器如何加载网站
## 网页包含什么
html:控制网页结构和内容
css:控制网页样式和布局
js:控制网页行为和交互
以及其他资源如图片、视频等。浏览器在加载网站时，会先解析 HTML 文件，然后根据 HTML 中的链接加载 CSS 文件和 JavaScript 文件，最后渲染网页内容。
## 渲染过程
浏览器渲染网页的过程大致如下：
1. 解析 HTML：浏览器读取 HTML 文件，构建 DOM（Document Object Model）树。
2. 解析 CSS：浏览器读取 CSS 文件，构建 CSSOM（CSS Object Model）树。
3. 执行 JavaScript：浏览器执行 JavaScript 代码，可能会修改 DOM 和 CSSOM。
4. 构建渲染树：浏览器根据 DOM 和 CSSOM 构建渲染树。
5. 布局：浏览器计算渲染树中每个节点的几何信息。
6. 绘制：浏览器将渲染树绘制到屏幕上。

# 如何连接HTML和CSS
通常我们在 HTML 文件中使用 `<link>` 标签来连接 CSS 文件。示例如下：
```html
<link rel="stylesheet" type="text/css" href="styles.css">
```
其中，`rel` 属性指定链接的关系，这里是样式表；`type` 属性指定链接的类型，这里是 CSS；`href` 属性指定 CSS 文件的路径。

# 层叠、优先级与继承
## 冲突规则
创建了两个应用于同一个元素的规则

优先级：决定发生冲突时先用哪条规则
```
!important > 行内样式 > ID > 类/属性/伪类 > 标签 > 继承
```
## 层叠
即顺序，同等优先级，靠后的是实际使用的规则
## 继承
一些设置在父元素上的css属性可以被子元素继承

# CSS 选择器
CSS 选择器用于选择 HTML 元素，以便应用样式。常见的选择器类型包括：
1. **元素选择器**：选择特定的 HTML 元素。
   ```css
   p {
       color: blue;
   }
   ```  
2. **类选择器**：选择具有特定类的元素。
   ```css
   .my-class {
       color: red;
   }
   ```  
3. **ID 选择器**：选择具有特定 ID 的元素。
   ```css
   #my-id {
       color: green;
   }
   ```
4. **组合选择器**：选择特定关系的元素。
   - **后代选择器**：选择某个元素内部的所有指定元素。
   ```css
   .my-class p {
       color: blue;
   }
   ```
    - **子选择器**：选择某个元素的直接子元素。
    ```css
    .my-class > p {
          color: blue;
     }
     ```
5. **分组选择器**：同时选择多个元素。
   ```css
   h1, h2, h3 {
       font-family: 'Helvetica', sans-serif;
   }
   ```
6. **选择器P**:样式化所有段落。
    ```css
    p {
         font-size: 16px;
         line-height: 1.5;
    }
    ```
7. **属性选择器**:根据元素的属性值选择元素。
    ```css
    input[type="text"] {
         border: 1px solid #ccc;
    }
    ```
8. **伪类选择器**:选择元素的特定状态。
    ```css
    a:hover {
         color: red;
    }
    ```
hover伪类选择器用于选择鼠标悬停在链接上的状态，可以改变链接的颜色、背景等样式。

# 常用属性
## 文字样式
| 属性             | 作用   |
| -------------- | ---- |
| color          | 文字颜色 |
| font-size      | 字体大小 |
| font-weight    | 粗细   |
| text-align     | 水平对齐 |
| line-height    | 行高   |
| letter-spacing | 字间距  |
## 背景
| 属性             | 作用   |
| -------------- | ---- |
| background-color | 背景颜色 |
| background-image | 背景图片 |
| background-repeat | 背景重复 |
| background-size | 背景大小 |
| background-position | 背景位置 |

# 改变元素的默认样式
通常，浏览器会为 HTML 元素应用默认样式，这些样式可能会影响网页的外观。为了确保网页在不同浏览器中具有一致的外观，我们可以使用 CSS 来重置或覆盖这些默认样式。
# 使用类名
在 HTML 元素中使用 `class` 属性来指定类名，然后在 CSS 中使用类选择器来定义样式。
# 盒模型
盒模型是 CSS 中一个重要的概念，它定义了元素的布局和尺寸。每个 HTML 元素都可以看作是一个盒子，包含内容（content）、内边距（padding）、边框（border）和外边距（margin）。
# 处理不同方向的文本
# 溢出
# 值和单位
# 在css中调整大小
# 图片、媒体和表单元素
# 样式化表格