# RBT

奇怪，居然比 AVL 快？（难道是我 AVL 写慢了 QAQ）



### 接口

```c++
//typedef std::pair<const Key, T> valueType

bool insert(const valueType &data);

bool find(const Key &_keyData, T &val) const;

void del(const Key &_keyData);
```



### 进度

- [x] 搬运 + 修改



### 性能

百万级数据 随机字符串 `Key=std::string`  `Val=int`
| op     | time       |
| ------ | ---------- |
| insert | 5.687500s |
| find   | 0.750000s |
| del    | 0.703125s |