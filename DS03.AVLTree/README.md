# AVLTree



### 接口

支持基本的 CRUD 和基本信息显示

```c++
bool insert(const Key& key, const Val& val);

bool find(const Key& key, Val& val) const;

bool modify(const Key& key, const Val& val);

bool del(const Key& key);

size_t size() const;

void display() const;
```



### 进度

- [x] 代码结构
- [x] 插入、查询、修改
- [x] 删除



### 性能

百万级数据 顺序  `Key=int`  `Val=int`
| op     | time       |
| ------ | ---------- |
| insert | 1.859375s  |
| find   | 0.250000s |
| del    | 0.515625s |

百万级数据 随机字符串 `Key=std::string`  `Val=int`
| op     | time       |
| ------ | ---------- |
| insert | 6.500000s  |
| find   | 0.640625s |
| del    | 1.000000s |