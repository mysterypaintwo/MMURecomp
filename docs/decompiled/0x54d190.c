// FUN_0054d190 @ 0x54d190

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0054d190(void)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  float10 fVar5;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> abStack_70 [20];
  undefined4 uStack_5c;
  char *pcStack_58;
  undefined *local_54;
  int local_50;
  undefined4 local_4c;
  undefined4 local_48;
  int local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 *local_30;
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  undefined1 *local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_50 = 0x54d1a4;
  iVar2 = FUN_00507ec0();
  if (iVar2 != 2) {
    local_50 = 0x54d1b9;
    iVar2 = FUN_00507ec0();
    if (iVar2 != 4) {
      local_50 = 0x54d1c8;
      fVar5 = (float10)FUN_0043e550();
      if ((float10)_DAT_006c5614 < fVar5) {
        local_50 = 0x54d1dc;
        fVar5 = (float10)FUN_0043e550();
        _DAT_006c5614 = (float)(fVar5 + (float10)*(int *)(local_44 + 0x150));
      }
      local_50 = 0x54d1f6;
      cVar1 = FUN_00405360();
      if (cVar1 != '\0') {
        local_50 = 0x3f800000;
        local_54 = (undefined *)0x0;
        pcStack_58 = (char *)0x54d215;
        FUN_0043bc10();
        local_50 = _DAT_00697cf0;
        local_54 = (undefined *)0x0;
        pcStack_58 = (char *)0x54d22d;
        FUN_0043bc10();
        if (*(char *)(local_44 + 0x155) != '\0') {
          local_14 = 0x3f800000;
          local_10 = 0;
          local_c = _DAT_00697cf0;
          local_8 = 0;
        }
        local_18 = _DAT_006989ac;
        local_50 = local_10;
        local_54 = (undefined *)local_14;
        local_1c = abStack_70;
        local_48 = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (abStack_70,"SpikePlatformSpike");
        uVar3 = FUN_00401540(local_24,local_18);
        puVar4 = (undefined4 *)FUN_004014c0(local_2c,uVar3);
        FUN_005628a0(*puVar4,puVar4[1]);
        local_50 = local_8;
        local_54 = (undefined *)local_c;
        local_30 = abStack_70;
        local_4c = std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::
                   basic_string<char,std::char_traits<char>,std::allocator<char>_>
                             (abStack_70,"SpikePlatformSpike");
        uVar3 = FUN_00401540(local_38,local_18);
        puVar4 = (undefined4 *)FUN_004014c0(local_40,uVar3);
        FUN_005628a0(*puVar4,puVar4[1]);
        local_50 = 0;
        local_54 = (undefined *)0x0;
        pcStack_58 = "CrossShooterShot";
        uStack_5c = 0x54d316;
        FUN_004026b0();
        *(char *)(local_44 + 0x155) = '\x01' - (*(char *)(local_44 + 0x155) != '\0');
        local_50 = local_44 + 4;
        local_54 = &DAT_00675ed0;
        pcStack_58 = (char *)0x54d341;
        FUN_004405c0();
      }
    }
  }
  return;
}

