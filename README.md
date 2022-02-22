## 使用方法
convert.cpp内の 16, 17行目にある
```
std::string fontFilename = "input_font/xxxxxx.ttf";
std::string path = "input_char/ascii.txt";
```
を任意のフォント・文字列が入ったファイルに変更した後,
```
./run.sh
```
を実行してください(.TTFファイルは自前で用意してください).  
また, template.h内のHEIGHTを変更することで生成されるフォントの縦幅を変更できます.

## 使用目的
https://github.com/gfd-dennou-club/iotex-esp32-mrubyc のm5stack用のフォントを生成するために使います.  
生成後, main/font.hを生成されたfont.hに置き換えてください.