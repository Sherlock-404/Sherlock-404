# flex布局
弹性布局，用来为盒模型提供最大的灵活性
# 基本概念
采用Flex布局的元素，称为Flex容器，简称容器，他的所有子元素自动成为容器成员，称为Flex项目(flex item)
![alt text](image/flex/1785547545470.png)
容器默认存在两根轴：水平的主轴（main axis）和垂直的交叉轴（cross axis）。主轴的开始位置（与边框的交叉点）叫做main start，结束位置叫做main end；交叉轴的开始位置叫做cross start，结束位置叫做cross end。
项目默认沿主轴排列。单个项目占据的主轴空间叫做main size，占据的交叉轴空间叫做cross size。

# 容器的属性
## flex-direcrion
决定主轴方向（即项目的排列方向）
可能有四个值：
row（默认值）：主轴为水平方向，起点在左端。
row-reverse：主轴为水平方向，起点在右端。
column：主轴为垂直方向，起点在上沿。
column-reverse：主轴为垂直方向，起点在下沿。
## flex-wrap
有三个值：
* nowrap(默认)：不换行
* wrap:换行，第一行在上方
* wrap-reverse:换行，第一行在下方

## flex-flow
flex-flow属性是flex-direction属性和flex-wrap属性的简写形式，默认值为row nowrap

## justify-content
定义了项目在主轴上的对齐方式
可取五个值：
flex-start（默认值）：左对齐
flex-end：右对齐
center： 居中
space-between：两端对齐，项目之间的间隔都相等。
space-around：每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍。

## align-items
定义项目在交叉轴上如何对齐
可取五个值：
flex-start：交叉轴的起点对齐。
flex-end：交叉轴的终点对齐。
center：交叉轴的中点对齐。
baseline: 项目的第一行文字的基线对齐。
stretch（默认值）：如果项目未设置高度或设为auto，将占满整个容器的高度。

## align-content
定义了多根轴线的对齐方式，只有一根轴线时，该属性不起作用
可取六个值：
flex-start：与交叉轴的起点对齐。
flex-end：与交叉轴的终点对齐。
center：与交叉轴的中点对齐。
space-between：与交叉轴两端对齐，轴线之间的间隔平均分布。
space-around：每根轴线两侧的间隔都相等。所以，轴线之间的间隔比轴线与边框的间隔大一倍。
stretch（默认值）：轴线占满整个交叉轴。

# 项目的属性
## order
属性定义项目的排列顺序。数值越小，排列越靠前，默认为0

## flex-grow
定义项目的放大比例，默认0
（若为1，则等分剩余空间）

## flex-shrink
定义了项目的缩小比例，默认1，即若空间不足，该项目将缩小

## flex-basis
定义了在分配多余空间之前，项目占据的主轴空间（main size）。浏览器根据这个属性，计算主轴是否有多余空间。它的默认值为auto，即项目的本来大小

## flex
是flex-grow, flex-shrink 和 flex-basis的简写，默认值为0 1 auto。后两个属性可选
该属性有两个快捷值：auto (1 1 auto) 和 none (0 0 auto)。建议优先使用这个属性，而不是单独写三个分离的属性，因为浏览器会推算相关值。

## align-self
align-self属性允许单个项目有与其他项目不一样的对齐方式，可覆盖align-items属性。默认值为auto，表示继承父元素的align-items属性，如果没有父元素，则等同于stretch。
该属性可能取6个值，除了auto，其他都与align-items属性完全一致。