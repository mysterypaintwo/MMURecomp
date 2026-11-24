// FUN_004f7af0 @ 0x4f7af0

void __fastcall FUN_004f7af0(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  undefined1 *puVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  undefined *puVar8;
  undefined4 local_230;
  undefined4 local_22c;
  undefined4 local_228;
  undefined4 local_224;
  undefined4 local_220;
  undefined4 local_21c;
  undefined1 local_20c [24];
  undefined1 local_1f4 [24];
  undefined1 local_1dc [24];
  undefined1 local_1c4 [24];
  undefined1 local_1ac [16];
  undefined1 local_19c [16];
  undefined1 local_18c [16];
  undefined4 local_17c;
  undefined1 local_178 [16];
  undefined1 local_168 [16];
  undefined4 local_158;
  undefined4 local_144;
  undefined1 local_140 [16];
  undefined1 local_130 [16];
  undefined1 local_120 [16];
  undefined1 local_110 [16];
  undefined1 local_100 [16];
  undefined1 local_f0 [16];
  undefined4 local_e0;
  undefined1 local_dc [16];
  undefined1 local_cc [16];
  undefined1 local_bc [16];
  undefined1 local_ac [16];
  undefined1 local_9c [16];
  undefined4 local_8c;
  undefined1 local_88 [16];
  undefined1 local_78 [16];
  undefined4 local_68;
  undefined1 local_64 [16];
  undefined1 local_54 [16];
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  undefined1 local_34 [8];
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  int local_1c;
  uint local_18;
  int local_14;
  undefined1 local_10 [8];
  int local_8;
  
  for (local_8 = 0; local_8 < 0xd; local_8 = local_8 + 1) {
    FUN_00623fc0(param_1 + 0xf0 + local_8 * 0x98);
    FUN_00623fc0(param_1 + 0x8a8 + local_8 * 0xa0);
    FUN_00623fc0(param_1 + 0x10c8 + local_8 * 0xa0);
    FUN_00623fc0(param_1 + 0x18e8 + local_8 * 0x98);
    FUN_0043bc20((local_8 / 7) * 0x70 + 0x18,(local_8 % 7) * 0x10 + 0x20);
    puVar7 = local_10;
    puVar6 = local_24;
    FUN_00401930(puVar6,puVar7);
    uVar2 = FUN_004014c0(puVar6,puVar7);
    FUN_004026e0(uVar2);
    puVar8 = &DAT_006c6fb4;
    puVar7 = local_34;
    puVar6 = local_10;
    puVar5 = local_2c;
    FUN_00401930(puVar5,puVar6,puVar7,&DAT_006c6fb4);
    FUN_004014c0(puVar5,puVar6);
    uVar2 = FUN_004014c0(puVar7,puVar8);
    FUN_004026e0(uVar2);
    puVar8 = &DAT_006c6fb4;
    puVar7 = local_44;
    puVar6 = local_10;
    puVar5 = local_3c;
    FUN_00401930(puVar5,puVar6,puVar7,&DAT_006c6fb4);
    FUN_004014c0(puVar5,puVar6);
    uVar2 = FUN_004014c0(puVar7,puVar8);
    FUN_004026e0(uVar2);
    uVar3 = FUN_0061daf0(local_8);
    uVar3 = uVar3 & 0xff;
    FUN_00403240(uVar3);
    FUN_00403220(uVar3);
    cVar1 = FUN_00403d40();
    if (cVar1 == '\0') {
      uVar3 = FUN_0061daf0(local_8);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
      uVar3 = FUN_0061daf0(local_8);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
    }
    else {
      uVar2 = 0;
      FUN_00403240(0);
      FUN_00403220(uVar2);
      uVar2 = 0;
      FUN_00403240(0);
      FUN_00403220(uVar2);
      uVar3 = FUN_0061daf0(0);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
      uVar3 = FUN_0061daf0(0);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
      uVar3 = FUN_0061daf0(9);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
      uVar3 = FUN_0061daf0(9);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
      uVar3 = FUN_0061daf0(10);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
      uVar3 = FUN_0061daf0(10);
      uVar3 = uVar3 & 0xff;
      FUN_00403240(uVar3);
      FUN_00403220(uVar3);
    }
    cVar1 = FUN_00403d40();
    if (cVar1 == '\0') {
      uVar2 = FUN_0043bc20(0x10,0x98);
      puVar7 = local_64;
      FUN_00401930(puVar7,uVar2);
      uVar2 = FUN_004014c0(puVar7,uVar2);
      FUN_004026e0(uVar2);
    }
    else {
      uVar2 = FUN_0043bc20(0x9e,0xb3);
      puVar7 = local_54;
      FUN_00401930(puVar7,uVar2);
      uVar2 = FUN_004014c0(puVar7,uVar2);
      FUN_004026e0(uVar2);
    }
  }
  local_14 = 0;
  while( true ) {
    if (3 < local_14) goto LAB_004f8174;
    iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4);
    if (iVar4 <= local_14) break;
    FUN_00623fc0(param_1 + 0x2af0 + local_14 * 0x98);
    cVar1 = FUN_00403d40();
    if (cVar1 == '\0') {
      uVar2 = FUN_0043bc20(local_14 * 0x10 + 0x10,0xc0);
      puVar7 = local_88;
      FUN_00401930(puVar7,uVar2);
      local_21c = FUN_004014c0(puVar7,uVar2);
    }
    else {
      uVar2 = FUN_0043bc20(local_14 * 0x10 + 0x30,0xc0);
      puVar7 = local_78;
      FUN_00401930(puVar7,uVar2);
      local_21c = FUN_004014c0(puVar7,uVar2);
    }
    local_68 = local_21c;
    FUN_004026e0(local_21c);
    local_14 = local_14 + 1;
  }
  local_18 = 0;
  while (((int)local_18 < 3 &&
         (iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4), (int)local_18 < iVar4)))
  {
    FUN_00623fc0(param_1 + 0x3210 + local_18 * 0x98);
    cVar1 = FUN_00403d40();
    if (cVar1 == '\0') {
      uVar3 = local_18 & 0x80000001;
      if ((int)uVar3 < 0) {
        uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
      }
      uVar2 = FUN_0043bc20(local_14 * 0x10 + 0x10 + uVar3 * 8,((int)local_18 / 2) * 8 + 0xc0);
      puVar7 = local_ac;
      FUN_00401930(puVar7,uVar2);
      local_220 = FUN_004014c0(puVar7,uVar2);
    }
    else {
      uVar3 = local_18 & 0x80000001;
      if ((int)uVar3 < 0) {
        uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
      }
      uVar2 = FUN_0043bc20(local_14 * 0x10 + 0x30 + uVar3 * 8,((int)local_18 / 2) * 8 + 0xc0);
      puVar7 = local_9c;
      FUN_00401930(puVar7,uVar2);
      local_220 = FUN_004014c0(puVar7,uVar2);
    }
    local_8c = local_220;
    FUN_004026e0(local_220);
    local_18 = local_18 + 1;
  }
LAB_004f8174:
  local_1c = 0;
  while ((local_1c < 4 &&
         (iVar4 = CMFCPropertyGridProperty::GetSubItemsCount(DAT_006c55a4), local_1c < iVar4))) {
    FUN_00623fc0(param_1 + 0x2fb0 + local_1c * 0x98);
    uVar2 = FUN_0043bc20(local_1c * 0x10 + 0xa0,0x84);
    puVar7 = local_bc;
    FUN_00401930(puVar7,uVar2);
    uVar2 = FUN_004014c0(puVar7,uVar2);
    FUN_004026e0(uVar2);
    local_1c = local_1c + 1;
  }
  FUN_00623fc0(param_1 + 0x2a58);
  FUN_00623fc0(param_1 + 0x29c0);
  FUN_00623fc0(param_1 + 0x2928);
  FUN_00623fc0(param_1 + 0x2d50);
  FUN_00623fc0(param_1 + 0x2de8);
  FUN_00623fc0(param_1 + 0x2e80);
  FUN_00623fc0(param_1 + 0x2f18);
  FUN_00623fc0(param_1 + 0x20a0);
  FUN_00623fc0(param_1 + 0x2158);
  FUN_00623fc0(param_1 + 0x21f8);
  FUN_00623fc0(param_1 + 0x2298);
  FUN_00623fc0(param_1 + 0x2330);
  FUN_00623fc0(param_1 + 0x23e8);
  FUN_00623fc0(param_1 + 0x2480);
  FUN_00623fc0(param_1 + 0x2538);
  FUN_00623fc0(param_1 + 0x25d0);
  FUN_00623fc0(param_1 + 0x2688);
  FUN_00623fc0(param_1 + 0x2720);
  FUN_00623fc0(param_1 + 0x27d8);
  FUN_00623fc0(param_1 + 0x2870);
  uVar2 = FUN_0043bc20(0x88,0x84);
  puVar7 = local_cc;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x9e,0x81);
  puVar7 = local_dc;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  cVar1 = FUN_00403d40();
  if (cVar1 == '\0') {
    uVar2 = FUN_0043bc20(0xe,0xbd);
    puVar7 = local_100;
    FUN_00401930(puVar7,uVar2);
    local_224 = FUN_004014c0(puVar7,uVar2);
  }
  else {
    uVar2 = FUN_0043bc20(0x2e,0xbd);
    puVar7 = local_f0;
    FUN_00401930(puVar7,uVar2);
    local_224 = FUN_004014c0(puVar7,uVar2);
  }
  local_e0 = local_224;
  FUN_004026e0(local_224);
  uVar2 = FUN_0043bc20(0x30,0x70);
  puVar7 = local_110;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x40,0x70);
  puVar7 = local_120;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x50,0x70);
  puVar7 = local_130;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x60,0x70);
  puVar7 = local_140;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  cVar1 = FUN_00403d40();
  if (cVar1 == '\0') {
    local_228 = FUN_0043bc20(0x30,0xa8);
  }
  else {
    local_228 = FUN_0043bc20(0xc0,200);
  }
  local_144 = local_228;
  FUN_004026e0(local_228);
  cVar1 = FUN_00403d40();
  if (cVar1 == '\0') {
    uVar2 = FUN_0043bc20(0x10,0xb0);
    puVar7 = local_178;
    FUN_00401930(puVar7,uVar2);
    local_22c = FUN_004014c0(puVar7,uVar2);
  }
  else {
    uVar2 = FUN_0043bc20(0x58,0xb0);
    puVar7 = local_168;
    FUN_00401930(puVar7,uVar2);
    local_22c = FUN_004014c0(puVar7,uVar2);
  }
  local_158 = local_22c;
  FUN_004026e0(local_22c);
  cVar1 = FUN_00403d40();
  if (cVar1 == '\0') {
    uVar2 = FUN_0043bc20(0x10,0xb0);
    puVar7 = local_19c;
    FUN_00401930(puVar7,uVar2);
    local_230 = FUN_004014c0(puVar7,uVar2);
  }
  else {
    uVar2 = FUN_0043bc20(0x58,0xb0);
    puVar7 = local_18c;
    FUN_00401930(puVar7,uVar2);
    local_230 = FUN_004014c0(puVar7,uVar2);
  }
  local_17c = local_230;
  FUN_004026e0(local_230);
  uVar2 = FUN_0043bc20(0x68,0xb8);
  puVar7 = local_1ac;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x68,200);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x80,0xb8);
  puVar7 = local_1c4;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x80,200);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x98,0xb8);
  puVar7 = local_1dc;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0x98,200);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0xb0,0xb8);
  puVar7 = local_1f4;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0xb0,200);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0xe0,0xac);
  puVar7 = local_20c;
  FUN_00401930(puVar7,uVar2);
  uVar2 = FUN_004014c0(puVar7,uVar2);
  FUN_004026e0(uVar2);
  uVar2 = FUN_0043bc20(0xd6,200);
  FUN_004026e0(uVar2);
  cVar1 = FUN_00403d40();
  if (cVar1 != '\0') {
    uVar2 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar2);
    uVar2 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar2);
    uVar2 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar2);
    uVar2 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar2);
    uVar2 = 0;
    FUN_00403240(0);
    FUN_00403220(uVar2);
  }
  FUN_00441b70(0);
  FUN_004f7400();
  return;
}

