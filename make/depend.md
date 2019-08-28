# Makefileでの依存関係の書き方

## ソースファイルに対応する依存ファイル
```
SRC=a.cpp b.cpp main.cpp
DEP=$(SRC:.cpp=.d)
```
* cppに対応するdファイルとする

## コンパイラに依存関係を出力させる
```
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) -MMD -MP -MF $(@:.o=.d)
```
* `-MMD -MP`で依存関係の出力
* -MFでファイル名を指定(なくてもよい)

## 生成された依存ファイルを取り込む
```
-include $(DEP)
```

## 依存ファイルを消去しないようにする
```
.SECONDARY: $(DEP)
```
* 自動生成された中間ファイルが自動的に削除されて困ることがあるので明示する(簡単なものなら無くてもよい)

## 例

```
SRC=a.cpp b.cpp main.cpp
OBJ=$(SRC:.cpp=.o)
DEP=$(SRC:.cpp=.d)

TARGET=main
CFLAGS+=-O3 -Wall -Wextra -DNDEUBG -g

all: $(TARGET)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) -MMD -MP -MF $(@:.o=.d)

-include $(DEP)

$(TARGET): $(OBJ)
	$(CC) -o $@ $? $(LDFLAGS)

clean:
	$(RM) -rf $(OBJ) $(DEP) $(TARGET)

.PHONY: clean

.SECONDARY: $(DEP)
```
