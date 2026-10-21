# Hardware & Library Specs

## IR 38kHz + AM312 (ปัจจุบัน)
- TX PC4 ขาชิป14 → วงจรขับ IR LED Active HIGH มีตัวจำกัดกระแส, carrier38kHz duty33% (software SysTick)
- RX demodulated IR38kHz OUT Active LOW → PC4 ขาชิป14, โค้ดกลับ polarity; รุ่นจริงยังไม่ยืนยัน ไม่ใช้ photodiode เปล่า/obstacle sensor แทน
- VCC และระดับ OUT ของตัวรับต้องตรงกับ MCU ตามสเปกจริง ห้ามอิงไฟ RF เดิมและห้าม12V; GND ร่วมภายในแต่ละฝั่ง
- PIR PC1, LED PC2, servo PD2 เดิม ช่วง600–2380us เป้าหมาย160°
- ทดสอบ environment ir_test แทน rf_test; คอมไพล์ผ่าน ยังไม่ทดสอบ IR จริง

## RF 433MHz + AM312 (ประวัติเก่า)
- Transmitter LED PC2 ล่าสุด: เมื่อ PIR HIGH ติด 3s ดับพัก 3s แล้วรอ/เริ่มใหม่ถ้า HIGH; ไม่ gate การส่ง RF และไม่เปลี่ยน LED ใน rf_test
- Receiver ช่วงล่าสุด 600–2380µs เป้าหมายกวาดประมาณ 160° ทั้ง startup/คำสั่ง RF; home เปลี่ยน ต้องทดสอบไม่มีโหลด ระวังชนสุด/คราง ลดช่วงหากจำเป็น มุมจริงยังไม่ยืนยัน
- ยืนยันคู่ RF ส่ง–รับได้จริงหลังแก้สาย DATA TX เป็น PC4 (ขาชิป 14) ไม่ใช่ PC0 (ขาชิป 10); รับ A5C33C ซ้ำต่อเนื่อง
- โหมดปกติและ rf_test ใช้ decoder total/ratio เดียวกันแล้ว
- แก้ข้อมูลแรงดัน: ผู้ใช้ยืนยัน MCU ฝั่งรับใช้ 5V, RX DATA → PC4 ตรง ไม่ใช้ divider 10kΩ/18kΩ ตามสมมติฐาน 3.3V เดิม; ไม่ต่อ 3.3V โปรแกรมเมอร์ซ้อน VDD 5V
- `rf_test` ใช้พิน RF PC4 เดิมทั้งคู่; TX LED PC2 แสดงชุดส่ง RX PD2 LOW ไม่มี servo pulse โหมดปกติยังเลือกได้ตามเดิม
- Transmitter LED: PC2 (TSSOP20 ขา 12) Output Active High → R 330Ω → LED → GND ติดตาม PIR HIGH ดับเมื่อ LOW; ฝั่งรับไม่มี LED นี้
- MCU สองตัว CH32V003F4P6: โปรเจกต์ `transmitter/` และ `receiver/`
- ส่ง: AM312 OUT → PC1 input, ASK/OOK 433MHz TX DATA ← PC4 output
- รับ: MX-RM-5V DATA → PC4 input ผ่าน R divider 10kΩ/18kΩ เมื่อ DATA เป็น 5V และ MCU เป็น 3.3V; MG90S Signal ← PD2
- ฝั่งรับใช้ pulse 1000–2000µs คาดประมาณ 90°; ผลทดสอบเดิมจากผู้ใช้ 1000–1500µs ได้ประมาณ 45°
- ผังขาชิปและข้อควรระวังใน README ของแต่ละโฟลเดอร์ ห้ามเดาลำดับขาโมดูลจากภาพสินค้า

## Current Test: MG90S (โค้ด root เดิม)
- Servo: MG90S แบบกำหนดตำแหน่ง; เป้าหมายไป–กลับประมาณ 0–90°
- Signal: **PD2**, Output Push-Pull; PD1 คงไว้สำหรับ SDI
- Software PWM 50Hz, pulse 1000–1500µs; มุมจริงอาจต้องปรับเทียบ
- ไฟแยก 5V แนะนำอย่างน้อย 1A: แดง = +5V, น้ำตาล = GND, ส้ม/เหลือง = PD2
- ต่อ GND แหล่งจ่ายร่วมกับ MCU; ห้ามใช้ไฟ 3.3V ของ WCH-LinkE เลี้ยงเซอร์โว
- ผังขาชิปและการต่ออยู่ใน `project.md`; ยังไม่ยืนยันผลทดสอบจริง

## Hardware

| Part | Model | Interface | Notes |
|---|---|---|---|
| MCU | CH32V003F4P6 | 1-Wire SDI (PD1) | 32-bit RISC-V QingKe V2A processor, Clock up to 48MHz, 16KB Flash, 2KB SRAM. TSSOP20 package. |
| Programmer | WCH-LinkE | USB to SWDIO (RISC-V) | อุปกรณ์ควบคุมการดีบั๊กและเบิร์นโปรแกรมอย่างเป็นทางการจาก WCH สำหรับไมโครคอนโทรลเลอร์รุ่น RISC-V |

### MCU CH32V003F4P6 Pinout Reference (TSSOP20)
- ขา **PD1**: SWDIO / SDI (Serial Debug Interface) - ใช้สำหรับอัปโหลดข้อมูลจาก WCH-LinkE
- ขา **PD6**: ขาเชื่อมต่อไฟ LED ทั่วไปสำหรับทดสอบบอร์ด (มักพบเป็นไฟบนบอร์ดในบางรุ่น)
- ขา **PA1, PA2**: ขาคริสตัลภายนอก หรือใช้เป็น GPIO ปกติได้หากใช้คล็อกภายใน (RC Oscillator 24MHz)
- แรงดันไฟเลี้ยงทำงาน: **2.7V - 5.5V** (แนะนำใช้ 3.3V จาก WCH-LinkE)

## Libraries

| Purpose | Library | Status | Notes |
|---|---|---|---|
| Core SDK | ch32v003fun | Active | เฟรมเวิร์ก C น้ำหนักเบาสำหรับการพัฒนา CH32V003 ทดแทน Standard Peripheral Library ขนาดเทอะทะ มีฟังก์ชันคุมรีจิสเตอร์และดีเลย์พื้นฐานพร้อมใช้ |

## Datasheet / References
- [CH32V003 Official Datasheet (WCH)](https://www.wch-ic.com/products/CH32V003.html)
- [ch32v003fun Github Repository](https://github.com/cnlohr/ch32v003fun)
- [Community-CH32V PlatformIO Github](https://github.com/Community-CH32V/platform-ch32v)

## Hardware Issues
- **WCH-LinkE Mode Check:**
  หากแฟลชโปรแกรมแล้วเจอปัญหาไม่พบอุปกรณ์ ให้ตรวจสอบก่อนว่าไฟบนตัว WCH-LinkE สว่างค้างเป็น **สีน้ำเงิน** (RISC-V Mode) หรือไม่ หากสว่างเป็น **สีแดง** (ARM Mode) จะต้องสลับโหมดก่อน โดยสามารถทำได้ผ่านซอฟต์แวร์ WCH Utility หรือการเสียบ USB ใหม่พร้อมกดปุ่ม Mode บนบอร์ด WCH-LinkE
- **1-Wire SDI Limitation:**
  เนื่องจาก CH32V003 ดีบั๊กด้วยสายสัญญาณเพียงเส้นเดียว (SDI) สัญญาณขานี้จึงมีความเปราะบางต่อสัญญาณรบกวนภายนอกสูงมาก แนะนำให้ใช้สายเชื่อมต่อที่สั้นและไม่พันกัน

## Test Notes
- วางแผนทำคำสั่งทดสอบไฟกะพริบที่ความถี่ 1Hz (เปิด 500ms / ปิด 500ms) ที่ขา PD6
