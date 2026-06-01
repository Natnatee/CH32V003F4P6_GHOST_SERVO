# CH32V003_CHECK

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


