# CH32V003_CHECK

## IR PIR + Servo: โค้ดปัจจุบัน
- เปลี่ยน transmitter/receiver จาก RF เป็น IR ทั้งสองฝั่ง PC4 ขาชิป14 เดิม; root ไม่เปลี่ยน
- TX IR LED ผ่านวงจรขับ Active HIGH, carrier ประมาณ38kHz 33%duty, absolute SysTick deadlines; PIR PC1 LED PC2 ติด3s/พัก3s และส่งตาม PIR เดิม
- RX ต้องเป็น demodulated IR38kHz OUT Active LOW ไม่ใช่ photodiode เปล่า/obstacle module; กลับ polarity ก่อน decoder และค่าเริ่มต้นทั้ง normal/test
- คงรหัสA5C33C decoder total/ratio ยืนยันสองแพ็กเก็ตใน500ms; servo600–2380us ประมาณ160° ค้าง5s/กลับ/พัก10s และ startup HOME1s/ออก/ค้าง/กลับ/READY เดิม
- Environment ปกติ ch32v003f4p6 และ test เปลี่ยนชื่อเป็น ir_test (-DIR_TEST=1); rf_test ไม่ใช่โหมดปัจจุบันแล้ว testTXส่งเอง testRXงดservo
- normal log [IR RX], test [IRTEST]/[US] mark=1 หมายถึงพาหะเข้า (ขาจริงLOW) ไม่ใช่ระดับPC4ตรง
- รุ่นโมดูล IR/วงจรขับจริงยังไม่ได้ยืนยัน ใช้สมมติฐาน38kHz OUT Active LOW/driver Active HIGH; ตรวจแรงดันและเกณฑ์ลอจิกตามรุ่น ห้ามจ่าย12Vเข้าชิป/IR LEDตรง
- Build ผ่าน normal/ir_test ทั้งสองฝั่ง; ยังไม่ได้แฟลชหรือยืนยัน IR บนฮาร์ดแวร์
- วิธีแฟลช/ต่อสายล่าสุดใน README ของแต่ละโฟลเดอร์ ประวัติ RF ด้านล่างไม่ใช่ configuration ปัจจุบัน

## RF PIR + Servo: ประวัติก่อนเปลี่ยนเป็น IR
- Transmitter LED PC2 โหมดปกติ: PIR HIGH เริ่มติด 3s แล้วดับพัก 3s ไม่ retrigger ระหว่างรอบ พักครบถ้ายัง HIGH เริ่มใหม่; RF ส่งตาม PIR เหมือนเดิม ไม่มี delay 3s บล็อก RF (LED อาจคลาดเคลื่อนราว 80ms ตามเวลาส่งแพ็กเก็ต) rf_test ไม่เปลี่ยน
- ปรับ receiver โหมดปกติกวาดประมาณ 160°: HOME=600µs OUT=2380µs STEP=10µs/20ms ขาละประมาณ 3.56s ทั้ง startup/คำสั่ง RF; home เปลี่ยนจากเดิม ค้าง 5s/พัก 10s เดิม ต้องทดสอบไม่มีโหลดและลดช่วงหากชนสุด ยังไม่ยืนยันมุมใหม่จริง
- ยืนยัน RF สำเร็จ: DATA ตัวส่งเดิมต่อผิด PC0 ขาชิป 10; ย้ายเป็น PC4 ขาชิป 14 แล้วรับ A5C33C ต่อเนื่อง packets=157 match=69 maxbits=24 ตาม log ผู้ใช้
- โหมดปกติ receiver ใช้ decoder แบบ total/ratio ที่ทดสอบ RF ผ่านแล้ว; log เฉพาะเปลี่ยนสถานะ/CONFIRMED ไม่สรุป noise ทุกวินาที
- Startup โหมดปกติ: HOME 1000µs 1s → ค่อยออก 2000µs → HOLD 5s → ค่อยกลับ → READY; หลังคำสั่ง RF ยังคงออก/ค้าง 5s/กลับ/พัก 10s
- การทำงานรวม PIR + RF + servo หลังอัปเดตยังต้องทดสอบบนบอร์ดจริง; rf_test ยังอยู่
- เพิ่ม environment `rf_test` ทั้ง transmitter/receiver ค่า default ยังเป็น `ch32v003f4p6` (โหมดเดิม)
- RF test decoder ทดลอง total HIGH+LOW 900–1500µs, pulse แต่ละด้าน 80–1300µs, ด้านสั้น ≤80% ด้านยาว; ยังคงตรวจรหัส A5C33C ซ้ำสองแพ็กเก็ตใน 500ms โหมดปกติไม่เปลี่ยนเกณฑ์ ต้องยืนยันฮาร์ดแวร์จริง
- RF test ส่งอัตโนมัติชุดละ 8 แพ็กเก็ต พัก 1s ไม่อ่าน PIR; รับงด servo (PD2 LOW), log timing H/L, pulse histogram, failH/failL/bits/maxbits และตัวนับแพ็กเก็ตผ่าน SDI
- Build ผ่านทั้ง 4 ชุด (2 บอร์ด × 2 environments); ยังไม่ได้แฟลชหรือทดสอบ RF mode บนฮาร์ดแวร์
- ผู้ใช้ยืนยัน MCU ฝั่งรับใช้ 5V: RF DATA 5V → PC4 ตรง ไม่ใช้ divider 10k/18k ของคำแนะนำเก่า; ห้ามต่อไฟ 3.3V โปรแกรมเมอร์ซ้อน VDD 5V
- ผลตรวจล่าสุด: PC4 ต่อ GND อ่าน 0 นิ่ง edges=0; ต่อ RF พบ edges และ sync/bad ทั้งตอนปิดตัวส่ง แต่ packets=0 ยังยืนยันไม่ได้ว่า RF เสีย
- Receiver startup test: สั่ง 2000µs ทันที (คาดประมาณ 90°), ค้าง 5s แล้วค่อยกลับ 1000µs และเข้า READY ไม่พัก 10s สำหรับ startup
- Receiver เปิด SDI debug log สรุปประมาณ 1s: state/pulse/PC4/edges/sync/bad/packets/last/match ผ่าน nonblocking DMDATA; ดูคำสั่งและความหมายใน `receiver/README.md` ยังไม่ทดสอบบนฮาร์ดแวร์จริง
- Transmitter เพิ่มไฟสถานะ PC2 (ขาชิป TSSOP20 หมายเลข 12) Output Active High ผ่าน R 330Ω และ LED ลง GND; ติดตาม PIR HIGH ดับเมื่อ LOW ฝั่งรับไม่มีไฟสถานะนี้
- `transmitter/`: CH32V003F4P6 บอร์ดขยาย, AM312 OUT → PC1 input, RF433 TX DATA ← PC4 output ส่งซ้ำเมื่อ PIR HIGH ไม่มี cooldown ฝั่งส่ง
- `receiver/`: เดฟบอร์ดเดิม, RF433 RX DATA → PC4 input ผ่านตัวแบ่งแรงดันหาก RX ใช้ 5V, MG90S Signal ← PD2 output
- ฝั่งรับ: ตรวจสองแพ็กเก็ต → หมุนออกประมาณ 90° → ค้าง 5s → กลับ → พัก 10s คำสั่งระหว่างรอบถูกทิ้ง ถ้า PIR ยัง HIGH หลังพักทำรอบใหม่ได้
- ใช้ pulse 1000–2000µs จากผลผู้ใช้ยืนยันว่า 1000–1500µs หมุนได้ประมาณ 45° ยังไม่ยืนยันช่วงใหม่หรือ RF จริง
- ผังขาชิป/สาย/แรงดัน/คำสั่ง build และ upload ดู `receiver/README.md` และ `transmitter/README.md`
- โค้ด root เดิมเก็บไว้ ไม่ได้เปลี่ยนตามโปรเจกต์ RF

## Current Test: MG90S Servo (โค้ด root เดิม)
- เฟิร์มแวร์ปัจจุบันทดสอบ MG90S ไป–กลับประมาณ 0–90° ที่ **PD2 (Output Push-Pull)** แทน Blink LED เดิม
- สัญญาณ software PWM 50Hz, pulse 1000–1500µs เพิ่ม/ลดทีละ 5µs ต่อเฟรม พักปลายทาง 500ms
- มุมจริงขึ้นกับเซอร์โว ต้องตรวจแบบไม่มีโหลดก่อน ไม่ขยายช่วง pulse หากชนสุดหรือคราง
- ใช้ blocking delay สำหรับการทดสอบเดี่ยวนี้ ห้ามเพิ่มงานที่บล็อกหรือ printf ในช่วงสร้าง pulse เพราะจะรบกวนสัญญาณ
- ใช้แหล่งจ่ายแยก 5V แนะนำอย่างน้อย 1A; GND ร่วมกับ MCU ห้ามเลี้ยงเซอร์โวจาก WCH-LinkE
- ยังไม่ยืนยันการทำงานจริงบนฮาร์ดแวร์สำหรับการทดสอบเซอร์โว

```text
CH32V003F4P6 (TSSOP20, มองด้านบน รอยบากอยู่บน)
           +---U---+
 PD4     1 |       | 20 PD3
 PD5     2 |       | 19 PD2 --> Signal MG90S (ส้ม/เหลือง)
 PD6     3 |       | 18 PD1 --> WCH-LinkE SDI
 PD7     4 |       | 17 PC7
 PA1     5 |       | 16 PC6
 PA2     6 |       | 15 PC5
 VSS     7 |       | 14 PC4
 PD0     8 |       | 13 PC3
 VDD     9 |       | 12 PC2
 PC0    10 |       | 11 PC1
           +-------+

ไฟแยก +5V ---------------- MG90S แดง
ไฟแยก GND ----+----------- MG90S น้ำตาล
MCU VSS ------+
```
ดูชื่อ PD2 บนบอร์ดเป็นหลัก; ผังด้านบนเป็นขาชิป ไม่ใช่ลำดับ header ของบอร์ด

> หมายเหตุ: บันทึก Blink ด้านล่างเป็นประวัติเดิม และข้อความเดิมที่ระบุ PC2 = Pin 9 ไม่ถูกต้อง สำหรับ TSSOP20 PC2 คือขา 12

## Overview
โปรเจกต์นี้ตั้งขึ้นเพื่อตรวจสอบและเซ็ตอัพโครงสร้างการพัฒนาไมโครคอนโทรลเลอร์ CH32V003F4P6 (สถาปัตยกรรม RISC-V 32-bit จากค่าย WCH) โดยเชื่อมต่อกับคอมพิวเตอร์ผ่านตัวโปรแกรมเมอร์ WCH-LinkE (ต่อแบบ 3 สาย: VCC, GND, SWDIO)

## Goals
- ทดสอบความถูกต้องในการต่อสายระหว่าง CH32V003F4P6 กับ WCH-LinkE
- คอมไพล์และอัปโหลดเฟิร์มแวร์ไฟกระพริบ (Blink LED) ผ่าน PlatformIO CLI ด้วยเฟรมเวิร์ก `ch32v003fun`
- ยืนยันว่าไมโครคอนโทรลเลอร์ทำงานและตอบสนองได้ตามต้องการ

## Hardware
- **MCU:** CH32V003F4P6 (TSSOP20, Flash 16KB, RAM 2KB)
- **Programmer:** WCH-LinkE (ในโหมด RISC-V SDI Mode)
- **LED:** ต่อภายนอกที่ขา PC2 (Pin 9 ของชิป) -> ทดสอบทำงานได้สำเร็จ 100%

## Pin Map

| Function | Pin | Device | Direction | Notes |
|---|---|---|---|---|
| SWDIO / SDI | PD1 | WCH-LinkE | Bidirectional | ขาโปรแกรมและดีบั๊กแบบ 1-Wire Serial Debug Interface |
| LED | PC2 | LED | Output | ขาสำหรับทดสอบ Blink (ไฟกระพริบ) Active High (ใช้งานได้ปกติ) |
| VCC | VCC (3.3V) | WCH-LinkE | Power Input | จ่ายไฟ 3.3V เลี้ยงวงจร |
| GND | GND | WCH-LinkE | Ground Reference | กราวด์ร่วมกัน |

## Electrical / Safety Notes
- ขา SWDIO/SDI (PD1) มีวงจรดึงสถานะภายในบอร์ด แนะนำไม่ให้ต่อโหลดอื่นที่มีความต้านทานต่ำพ่วงกับขานี้ขณะอัปโหลดเฟิร์มแวร์
- ตรวจสอบให้แน่ใจว่า WCH-LinkE ทำงานในโหมด RISC-V (ไฟ LED สีน้ำเงินสว่างค้าง หรือกระพริบสั้นๆ ไม่ใช่โหมด ARM สีแดง)
- ห้ามจ่ายไฟเลี้ยงจากพอร์ตอื่นซ้อนกับไฟ 3.3V ที่มาจาก WCH-LinkE เพื่อป้องกันความเสียหาย

## Firmware Architecture
- โครงสร้างโปรเจกต์อิงตามสถาปัตยกรรมของเฟรมเวิร์ก `ch32v003fun`
- โค้ดหลักเขียนใน `src/main.c` โดยตรงเพื่อความกระชับ
- ใช้คำสั่งระบบเริ่มต้นและกำหนดความเร็วคล็อกผ่าน `SystemInit()`

## Libraries
- **`ch32v003fun`** (Community-made lightweight SDK/Framework for CH32V003)

## Build & Upload
สามารถรันผ่าน PlatformIO CLI บน Terminal ของผู้ใช้ด้วยคำสั่งดังต่อไปนี้:
- คลีนโปรเจกต์: `pio run --target clean`
- คอมไพล์โปรเจกต์: `pio run`
- คอมไพล์และอัปโหลด: `pio run -t upload`

⚠️ **ข้อควรระวังในการ Monitor:**
ห้ามใช้คำสั่ง `pio run -t upload -t monitor` บนบอร์ดนี้ เนื่องจาก PlatformIO จะพยายามเรียกใช้มอนิเตอร์ผ่าน Serial COM Port แบบเดิมซึ่งคอมพอร์ตมักหายบน Windows 
**ให้ใช้คำสั่งดักฟังแยกหน้าต่างแทนด้วย:**
```bash
/c/Users/natna/.platformio/packages/tool-minichlink/minichlink -T
```

## Current State
- [x] เชื่อมต่อสาย VCC, GND, SWDIO ระหว่าง WCH-LinkE และ CH32V003F4P6
- [x] สร้างโครงสร้างไฟล์โปรเจกต์พื้นฐาน (`rule.md`, `project.md`, `platformio.ini`, `src/main.c` ฯลฯ)
- [x] คอมไพล์เฟิร์มแวร์สำเร็จเป็นครั้งแรก (ใช้ Flash เพียง 476 ไบต์!)
- [x] อัปโหลดเฟิร์มแวร์ลงบนชิปสำเร็จเป็นครั้งแรก (ตรวจจับบอร์ด CH32V003 ได้ถูกต้อง)
- [x] ทดสอบสแกนหาพินและควบคุมไฟกระพริบจริงที่พิน **PC2** สำเร็จเรียบร้อย (ไฟสว่างติดค้างและกะพริบตามคำสั่งปกติ)

## Issue Log
- **WLink Open Error / libusb_open() = -5 (2026-05-22):** คอมไพล์ผ่านแต่ไม่สามารถอัปโหลดเฟิร์มแวร์ได้ เนื่องจาก WCH-LinkE อยู่ในโหมด ARM หรือมีปัญหาไดรเวอร์ WinUSB บน Windows
  - *วิธีแก้ไข:* ใช้โปรแกรม WCH-LinkUtility ในการสลับโหมดเป็น RISC-V (WCH-LinkRV) และใช้ Zadig เปลี่ยนไดรเวอร์ WCH-Link (โหมด RISC-V) ให้เป็น WinUSB สำเร็จเรียบร้อย!


