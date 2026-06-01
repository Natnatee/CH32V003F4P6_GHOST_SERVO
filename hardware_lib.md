# Hardware & Library Specs

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
