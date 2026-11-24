// FUN_0050e610 @ 0x50e610

void __thiscall FUN_0050e610(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  void *pvVar2;
  undefined4 local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  undefined4 local_1e0;
  undefined4 local_1dc;
  undefined4 local_1d8;
  undefined4 local_1d4;
  undefined4 local_1d0;
  undefined4 local_1cc;
  undefined4 local_1c8;
  undefined4 local_1c4;
  undefined4 local_1c0;
  undefined4 local_1bc;
  undefined4 local_1b8;
  undefined4 local_1b4;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  undefined4 local_19c;
  undefined4 local_198;
  undefined4 local_194;
  undefined4 local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined4 local_15c;
  undefined4 local_158;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_006411da;
  local_10 = ExceptionList;
  uVar1 = DAT_006c1e94 ^ (int)register0x00000010 - 4U;
  ExceptionList = &local_10;
  param_1[0xf] = param_2;
  if (param_1[8] != 0) {
    if ((undefined4 *)param_1[8] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[8])(1,uVar1);
    }
    param_1[8] = 0;
  }
  switch(param_2) {
  case 1:
    pvVar2 = operator_new(0xfc);
    local_8 = 5;
    if (pvVar2 == (void *)0x0) {
      local_16c = 0;
    }
    else {
      local_16c = FUN_00528420();
    }
    param_1[8] = local_16c;
    *param_1 = 1;
    break;
  case 2:
    pvVar2 = operator_new(0xfc);
    local_8 = 3;
    if (pvVar2 == (void *)0x0) {
      local_164 = 0;
    }
    else {
      local_164 = FUN_00538790();
    }
    param_1[8] = local_164;
    *param_1 = 1;
    break;
  case 3:
    pvVar2 = operator_new(0x104);
    local_8 = 7;
    if (pvVar2 == (void *)0x0) {
      local_174 = 0;
    }
    else {
      local_174 = FUN_00531b80();
    }
    param_1[8] = local_174;
    *param_1 = 1;
    break;
  case 4:
    pvVar2 = operator_new(0x100);
    local_8 = 2;
    if (pvVar2 == (void *)0x0) {
      local_160 = 0;
    }
    else {
      local_160 = FUN_0052b850();
    }
    param_1[8] = local_160;
    *param_1 = 1;
    break;
  case 5:
    pvVar2 = operator_new(0xfc);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_158 = 0;
    }
    else {
      local_158 = FUN_00512a90();
    }
    param_1[8] = local_158;
    *param_1 = 1;
    break;
  case 6:
    pvVar2 = operator_new(0xfc);
    local_8 = 1;
    if (pvVar2 == (void *)0x0) {
      local_15c = 0;
    }
    else {
      local_15c = FUN_005369e0();
    }
    param_1[8] = local_15c;
    *param_1 = 1;
    break;
  case 7:
    pvVar2 = operator_new(0xfc);
    local_8 = 6;
    if (pvVar2 == (void *)0x0) {
      local_170 = 0;
    }
    else {
      local_170 = FUN_0052ec10();
    }
    param_1[8] = local_170;
    *param_1 = 1;
    break;
  case 8:
    pvVar2 = operator_new(0x100);
    local_8 = 4;
    if (pvVar2 == (void *)0x0) {
      local_168 = 0;
    }
    else {
      local_168 = FUN_005341c0();
    }
    param_1[8] = local_168;
    *param_1 = 1;
    break;
  case 9:
    pvVar2 = operator_new(0xfc);
    local_8 = 8;
    if (pvVar2 == (void *)0x0) {
      local_178 = 0;
    }
    else {
      local_178 = FUN_00515dc0();
    }
    param_1[8] = local_178;
    *param_1 = 1;
    break;
  case 10:
    pvVar2 = operator_new(0x108);
    local_8 = 10;
    if (pvVar2 == (void *)0x0) {
      local_180 = 0;
    }
    else {
      local_180 = FUN_0051d1f0();
    }
    param_1[8] = local_180;
    *param_1 = 1;
    break;
  case 0xb:
    pvVar2 = operator_new(0x100);
    local_8 = 0xb;
    if (pvVar2 == (void *)0x0) {
      local_184 = 0;
    }
    else {
      local_184 = FUN_00518750();
    }
    param_1[8] = local_184;
    *param_1 = 1;
    break;
  case 0xc:
    pvVar2 = operator_new(0x100);
    local_8 = 0xc;
    if (pvVar2 == (void *)0x0) {
      local_188 = 0;
    }
    else {
      local_188 = FUN_0051af80();
    }
    param_1[8] = local_188;
    *param_1 = 1;
    break;
  case 0xd:
    pvVar2 = operator_new(0x128);
    local_8 = 0xd;
    if (pvVar2 == (void *)0x0) {
      local_18c = 0;
    }
    else {
      local_18c = FUN_005211f0();
    }
    param_1[8] = local_18c;
    *param_1 = 1;
    break;
  case 0xe:
    pvVar2 = operator_new(0x228);
    local_8 = 0xe;
    if (pvVar2 == (void *)0x0) {
      local_190 = 0;
    }
    else {
      local_190 = FUN_00525700();
    }
    param_1[8] = local_190;
    *param_1 = 1;
    break;
  case 0x10:
    pvVar2 = operator_new(300);
    local_8 = 0xf;
    if (pvVar2 == (void *)0x0) {
      local_194 = 0;
    }
    else {
      local_194 = FUN_0060c4e0();
    }
    param_1[8] = local_194;
    *param_1 = 1;
    break;
  case 0x11:
    pvVar2 = operator_new(300);
    local_8 = 0x10;
    if (pvVar2 == (void *)0x0) {
      local_198 = 0;
    }
    else {
      local_198 = FUN_00601040();
    }
    param_1[8] = local_198;
    *param_1 = 1;
    break;
  case 0x12:
    pvVar2 = operator_new(300);
    local_8 = 0x11;
    if (pvVar2 == (void *)0x0) {
      local_19c = 0;
    }
    else {
      local_19c = FUN_005fa010();
    }
    param_1[8] = local_19c;
    *param_1 = 1;
    break;
  case 0x13:
    pvVar2 = operator_new(0xfc);
    local_8 = 9;
    if (pvVar2 == (void *)0x0) {
      local_17c = 0;
    }
    else {
      local_17c = FUN_005e33a0();
    }
    param_1[8] = local_17c;
    *param_1 = 1;
    break;
  case 0x14:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x12;
    if (pvVar2 == (void *)0x0) {
      local_1a0 = 0;
    }
    else {
      local_1a0 = FUN_005e7e30();
    }
    param_1[8] = local_1a0;
    *param_1 = 1;
    break;
  case 0x15:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x13;
    if (pvVar2 == (void *)0x0) {
      local_1a4 = 0;
    }
    else {
      local_1a4 = FUN_005e6ae0();
    }
    param_1[8] = local_1a4;
    *param_1 = 1;
    break;
  case 0x16:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x14;
    if (pvVar2 == (void *)0x0) {
      local_1a8 = 0;
    }
    else {
      local_1a8 = FUN_005e85e0();
    }
    param_1[8] = local_1a8;
    *param_1 = 1;
    break;
  case 0x17:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x15;
    if (pvVar2 == (void *)0x0) {
      local_1ac = 0;
    }
    else {
      local_1ac = FUN_005e8f30();
    }
    param_1[8] = local_1ac;
    *param_1 = 1;
    break;
  case 0x18:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x16;
    if (pvVar2 == (void *)0x0) {
      local_1b0 = 0;
    }
    else {
      local_1b0 = FUN_005e6160();
    }
    param_1[8] = local_1b0;
    *param_1 = 1;
    break;
  case 0x19:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x17;
    if (pvVar2 == (void *)0x0) {
      local_1b4 = 0;
    }
    else {
      local_1b4 = FUN_005e7460();
    }
    param_1[8] = local_1b4;
    *param_1 = 1;
    break;
  case 0x1a:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x18;
    if (pvVar2 == (void *)0x0) {
      local_1b8 = 0;
    }
    else {
      local_1b8 = FUN_005e44e0();
    }
    param_1[8] = local_1b8;
    *param_1 = 1;
    break;
  case 0x1b:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x19;
    if (pvVar2 == (void *)0x0) {
      local_1bc = 0;
    }
    else {
      local_1bc = FUN_005e98b0();
    }
    param_1[8] = local_1bc;
    *param_1 = 1;
    break;
  case 0x1c:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x1a;
    if (pvVar2 == (void *)0x0) {
      local_1c0 = 0;
    }
    else {
      local_1c0 = FUN_005e4e60();
    }
    param_1[8] = local_1c0;
    *param_1 = 1;
    break;
  case 0x1d:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x1b;
    if (pvVar2 == (void *)0x0) {
      local_1c4 = 0;
    }
    else {
      local_1c4 = FUN_005e57e0();
    }
    param_1[8] = local_1c4;
    *param_1 = 1;
    break;
  case 0x1e:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x1c;
    if (pvVar2 == (void *)0x0) {
      local_1c8 = 0;
    }
    else {
      local_1c8 = FUN_005edbd0();
    }
    param_1[8] = local_1c8;
    *param_1 = 1;
    break;
  case 0x1f:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x1d;
    if (pvVar2 == (void *)0x0) {
      local_1cc = 0;
    }
    else {
      local_1cc = FUN_005ec880();
    }
    param_1[8] = local_1cc;
    *param_1 = 1;
    break;
  case 0x20:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x1e;
    if (pvVar2 == (void *)0x0) {
      local_1d0 = 0;
    }
    else {
      local_1d0 = FUN_005ee380();
    }
    param_1[8] = local_1d0;
    *param_1 = 1;
    break;
  case 0x21:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x1f;
    if (pvVar2 == (void *)0x0) {
      local_1d4 = 0;
    }
    else {
      local_1d4 = FUN_005eecd0();
    }
    param_1[8] = local_1d4;
    *param_1 = 1;
    break;
  case 0x22:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x20;
    if (pvVar2 == (void *)0x0) {
      local_1d8 = 0;
    }
    else {
      local_1d8 = FUN_005ebf00();
    }
    param_1[8] = local_1d8;
    *param_1 = 1;
    break;
  case 0x23:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x21;
    if (pvVar2 == (void *)0x0) {
      local_1dc = 0;
    }
    else {
      local_1dc = FUN_005ed200();
    }
    param_1[8] = local_1dc;
    *param_1 = 1;
    break;
  case 0x24:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x22;
    if (pvVar2 == (void *)0x0) {
      local_1e0 = 0;
    }
    else {
      local_1e0 = FUN_005ea280();
    }
    param_1[8] = local_1e0;
    *param_1 = 1;
    break;
  case 0x25:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x23;
    if (pvVar2 == (void *)0x0) {
      local_1e4 = 0;
    }
    else {
      local_1e4 = FUN_005ef650();
    }
    param_1[8] = local_1e4;
    *param_1 = 1;
    break;
  case 0x26:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x24;
    if (pvVar2 == (void *)0x0) {
      local_1e8 = 0;
    }
    else {
      local_1e8 = FUN_005eac00();
    }
    param_1[8] = local_1e8;
    *param_1 = 1;
    break;
  case 0x27:
    pvVar2 = operator_new(0xfc);
    local_8 = 0x25;
    if (pvVar2 == (void *)0x0) {
      local_1ec = 0;
    }
    else {
      local_1ec = FUN_005eb580();
    }
    param_1[8] = local_1ec;
    *param_1 = 1;
  }
  DAT_006c55f0 = param_1[8];
  ExceptionList = local_10;
  return;
}

