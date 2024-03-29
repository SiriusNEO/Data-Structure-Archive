# B-Tree

设计思路：

- 大体按 B 树算法
  - 插入：找到块，然后如果太多分裂
  - 删除：问题归结为删除叶子节点，删除后块大小低于下限尝试借或者合并
  - 查询：直接找
- 哈希：采用 `std::hash`  将 `key`  值哈希，加快比较速度
- 内存回收：开一个栈，存空闲位置，超出空闲位置的浪费掉
- cache：LRU cache



### 接口

支持基本的 CRUD 和基本信息显示

```c++
//find与display不声明为const原因是它们的read/write操作会更新cache

bool insert(const Key& key, const Val& val);

bool find(const Key& key, Val& val);

bool modify(const Key& key, const Val& val);

bool del(const Key& key);

size_t size();

void display();
```



### 问题

- ~~测试中 `del` 出现 bug~~ 2021.9.11 fixed

  发现的问题有：

  - 冗余代码（已进行简化，不过目前代码结构仍不算好看）
  - 非法文件读入（原因是 `disk read/write`  前没有判断合法）
  - `borrow from left/right`  需要修改 `parent`
  - 关于 `struct` 的成员是否按声明顺序储存的问题：应该是，在 `Linux g++`  以及 `Windows MinGW`  环境下均表现一致，不过此处对效率无明显影响



### 进度

- [x] 基本架构
- [x] 插入：单块
- [x] 插入：块分裂
- [x] 查询
- [x] 修改
- [x] 删除
- [x] cache



### 性能

百万随机数据（Key 为字符串，Val 为 int） 不开优化

| op     | time       |
| ------ | ---------- |
| insert | 15.015625s  |
| find   | 10.671875s |
| del    | 16.859375s |

-O3

| op     | time       |
| ------ | ---------- |
| insert | 5.359375s |
| find   | 2.953125s |
| del    | 4.281250s |

