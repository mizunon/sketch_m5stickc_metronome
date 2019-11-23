# sketch_m5stickc_metronome

## M5StickC viveration metronome.  
 By Takuhiro Mizuno
  
https://twitter.com/mizunon

## What is this?
This is M5StickC metronome with viberaion and light and click sound.

## Components
### M5StickC
https://m5stack.com/collections/m5-core/products/stick-c  
https://m5stack.com/collections/m5-core/products/m5stickc-development-kit-with-hat  

### Grove - Vibration Motor. SKU:105020003
https://www.seeedstudio.com/Grove-Vibration-Motor.html  

### M5StickC Speaker Hat
Include in 'M5StickC+ Development Kit with Hat'.  
https://m5stack.com/products/m5stickc-speaker-hat  

## Usage
### BtnA('M5')
Start/Stop metronome.  

### PowerBtn(upper side)
Tempo Up or Volume Up.  
Long press(over 6s) to Power off.

### BtnB(lower side)
Tempo down or Volume Down.  
Long press to change tempo cursol.  

## これは何？
M5StickCを使った、メトロノームです。光と振動、音でビートを教えてくれます。  
ただし、拍子機能はありません。  

## コンポーネント
### M5StickC
https://m5stack.com/collections/m5-core/products/stick-c  
https://m5stack.com/collections/m5-core/products/m5stickc-development-kit-with-hat  

### Grove - Vibration Motor. SKU:105020003
M5StickCのGroveポートに接続するとビートを振動で伝えてくれます。  
https://www.seeedstudio.com/Grove-Vibration-Motor.html  

### M5StickC Speaker Hat
M5StickCのI/Oポートに接続するとビートを音で伝えてくれます。
Include in 'M5StickC+ Development Kit with Hat'.  
https://m5stack.com/products/m5stickc-speaker-hat  

## Usage
### ボタンA('M5'って書いてあるやつ)
メトロノームをスタート、ストップします.  

### 電源ボタン(上側のボタン)
テンポを上げる（停止中）、音量を大きくする（再生中）。  
6秒以上長押しすると電源が切れます（M5StickCの標準機能）。

### ボタンB（下側のボタン）
テンポを下げる（停止中）、音量を小さくする（再生中）。  
2秒以上長押しすると、テンポを変更する桁を変えることができます（1の位→10の位→100の位→1の位）

### 光と音量
音量を最小まで下げると、音も光も消えます。そこから１段階上げると、光のみ。さらに１段上げると光と音の両方になります。
