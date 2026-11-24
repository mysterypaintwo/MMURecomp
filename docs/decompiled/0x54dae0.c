// FUN_0054dae0 @ 0x54dae0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0054dae0(int *param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> *this;
  char *_Str1;
  undefined4 *puVar6;
  int *piVar7;
  float *pfVar8;
  float10 fVar9;
  char *_Str2;
  char *local_8c;
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  int local_54;
  int local_50;
  undefined4 local_4c;
  undefined4 local_48;
  int local_44;
  int local_40;
  char local_3c [4];
  undefined1 local_38 [16];
  int local_28;
  undefined4 local_24;
  char local_20;
  int local_1c;
  int local_18;
  int local_14;
  char local_d;
  int local_c;
  char local_5;
  
  iVar4 = FUN_00507ec0();
  if (iVar4 != 2) {
    if ((float)param_1[0x57] == (float)_DAT_00697ce0) {
      param_1[0x19] = (int)-(float)param_1[0x5a];
      FUN_00401510(param_1 + 0x16);
      local_24 = FUN_0041efa0();
      local_28 = param_1[0x16];
      local_20 = '\x01';
      FUN_0043eef0(10);
      if (local_20 == '\0') {
        FUN_004405c0(&DAT_00675f54,param_1 + 1);
        return;
      }
      uVar5 = (**(code **)(*param_1 + 4))(&DAT_00675f5c);
      (**(code **)(*param_1 + 0x10))(uVar5);
    }
    if (param_1[0x59] != 0) {
      _Str2 = "Death";
      this = (basic_string<char,std::char_traits<char>,std::allocator<char>_> *)FUN_0043d890();
      _Str1 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::c_str(this);
      iVar4 = strcmp(_Str1,_Str2);
      if (iVar4 == 0) {
        param_1[0x59] = 0;
      }
    }
    local_18 = param_1[0x16];
    local_14 = param_1[0x17];
    FUN_0043cb00();
    local_5 = '\x01';
    local_d = '\0';
    fVar1 = (float)param_1[0x2c];
    fVar2 = (float)_DAT_00697ce0;
    if (((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) ||
       (iVar4 = FUN_0043cb00(), (float)_DAT_00698c10 <= *(float *)(iVar4 + 4))) {
      local_5 = '\0';
      if (((float)param_1[0x19] < (float)_DAT_00697ce0) &&
         ((float)param_1[0x2c] < (float)_DAT_00698c60)) {
        local_d = '\x01';
      }
      if (((float)_DAT_00697ce0 < (float)param_1[0x19]) &&
         ((float)_DAT_00698c58 < (float)param_1[0x2c])) {
        local_d = '\x01';
      }
    }
    if (local_d != '\0') {
      param_1[0x19] = (int)((float)param_1[0x19] * (float)_DAT_006987b8);
    }
    if (local_5 == '\0') {
      fVar9 = (float10)FUN_0043e510();
      uVar5 = FUN_00401540(local_5c,(float)fVar9);
      uVar5 = FUN_004014c0(local_64,uVar5);
      FUN_00401510(uVar5);
    }
    else {
      local_44 = param_1[0x18];
      local_40 = param_1[0x19];
      uVar5 = FUN_00401930(0x800,1,0,0);
      FUN_005b5220(local_3c,param_1,param_1 + 0x16,&local_44,uVar5);
      if (local_3c[0] == '\0') {
        param_1[0x19] = (int)((float)param_1[0x19] * (float)_DAT_006987b8);
      }
      FUN_00401510(local_38);
    }
    if (local_5 != '\0') {
      local_50 = 0;
      _String_const_iterator<>();
      puVar6 = (undefined4 *)FUN_00472660(local_6c);
      local_4c = *puVar6;
      local_48 = puVar6[1];
      uVar5 = FUN_0046df60(local_74);
      cVar3 = FUN_00426920(uVar5);
      if (cVar3 != '\0') {
        piVar7 = (int *)FUN_004bdc80();
        local_54 = *piVar7;
        if (*(int *)(local_54 + 8) == 0x800) {
          local_50 = local_50 + 1;
        }
                    /* WARNING: Subroutine does not return */
        FUN_0042f830();
      }
      if (((*(char *)((int)param_1 + 0x151) == '\0') && ((char)param_1[0x54] != '\0')) &&
         (local_50 == 1)) {
        *(undefined1 *)(param_1 + 0x54) = 0;
        *(undefined1 *)((int)param_1 + 0x151) = 1;
        if (param_1[0x59] != 0) {
          uVar5 = 0;
          puVar6 = (undefined4 *)FUN_00420a40(0);
          uVar5 = FUN_0043bc10(*puVar6,uVar5);
          FUN_00420160(uVar5);
          (**(code **)(*(int *)param_1[0x59] + 0xc))(9,0);
          param_1[0x59] = 0;
        }
        uVar5 = (**(code **)(*param_1 + 4))("Deactivate");
        (**(code **)(*param_1 + 0x10))(uVar5);
      }
      else if (((*(char *)((int)param_1 + 0x151) == '\0') && ((char)param_1[0x54] == '\0')) &&
              (local_50 == 2)) {
        *(undefined1 *)((int)param_1 + 0x151) = 1;
        *(undefined1 *)(param_1 + 0x54) = 1;
        uVar5 = (**(code **)(*param_1 + 4))("Activate");
        (**(code **)(*param_1 + 0x10))(uVar5);
      }
    }
    if ((*(char *)((int)param_1 + 0x151) != '\0') && (cVar3 = FUN_00405360(), cVar3 != '\0')) {
      *(undefined1 *)((int)param_1 + 0x151) = 0;
      if ((char)param_1[0x54] == '\0') {
        local_8c = "NoStep";
      }
      else {
        local_8c = "Step";
      }
      uVar5 = (**(code **)(*param_1 + 4))(local_8c);
      (**(code **)(*param_1 + 0x10))(uVar5);
    }
    if ((char)param_1[0x54] != '\0') {
      FUN_00425d50(&local_c,&local_1c);
      if (param_1[0x59] == 0) {
        iVar4 = FUN_00420780();
        FUN_00551c10(param_1,&local_18,(float)local_1c,(float)iVar4);
      }
      else {
        pfVar8 = (float *)FUN_00401930();
        if ((float)param_1[0x16] - (float)local_c <= *pfVar8 + (float)_DAT_00698ad8) {
          pfVar8 = (float *)FUN_00401930();
          fVar1 = *pfVar8;
          fVar2 = (float)_DAT_00698ad8;
          iVar4 = FUN_00420780();
          if (fVar1 - fVar2 <= (float)(iVar4 - local_c) + (float)param_1[0x16]) {
            FUN_00551b60(param_1,param_1[0x59],(float)local_1c,_DAT_00675ee8);
            return;
          }
        }
        uVar5 = 0;
        puVar6 = (undefined4 *)FUN_00420a40(0);
        uVar5 = FUN_0043bc10(*puVar6,uVar5);
        FUN_00420160(uVar5);
        (**(code **)(*(int *)param_1[0x59] + 0xc))(9,0);
        param_1[0x59] = 0;
      }
    }
  }
  return;
}

