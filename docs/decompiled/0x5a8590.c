// FUN_005a8590 @ 0x5a8590

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005a8590(int *param_1)

{
  char cVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  float10 fVar6;
  undefined1 *puVar7;
  undefined4 uVar8;
  undefined1 local_34;
  char local_30;
  undefined1 local_28 [16];
  undefined1 local_18 [18];
  char local_6;
  undefined1 local_5;
  
  cVar1 = FUN_005a7900();
  if (cVar1 == '\0') {
    uVar2 = (**(code **)(*param_1 + 4))(&DAT_0067da24);
    (**(code **)(*param_1 + 0x10))(uVar2);
    cVar1 = FUN_00441db0();
    if ((cVar1 == '\0') && (((char)param_1[0x73] == '\0' || ((char)param_1[0x1a] != '\0')))) {
      local_30 = '\0';
    }
    else {
      local_30 = '\x01';
    }
    local_6 = local_30;
    cVar1 = FUN_00441db0();
    if ((cVar1 == '\0') && (((char)param_1[0x73] == '\0' || ((char)param_1[0x1a] == '\0')))) {
      local_34 = 0;
    }
    else {
      local_34 = 1;
    }
    local_5 = local_34;
    *(undefined1 *)(param_1 + 0x1a) = local_34;
    if ((param_1[0x26] & 0x10U) == 0) {
      if (local_6 == '\0') {
        fVar6 = (float10)FUN_0043e510();
        param_1[0x18] = (int)(float)(fVar6 * (float10)_DAT_00698340 + (float10)(float)param_1[0x18])
        ;
        uVar8 = 0;
        uVar3 = 1;
        uVar2 = FUN_0043bc20(1,0);
        puVar7 = local_28;
        FUN_0043cb00(puVar7,uVar2,uVar3,uVar8);
        puVar4 = (undefined4 *)FUN_004014c0(puVar7,uVar2);
        iVar5 = FUN_00445ea0(*puVar4,puVar4[1],uVar3,uVar8);
        if (iVar5 == 0) {
          param_1[0x16] = (int)((float)param_1[0x16] + (float)_DAT_00697ce8);
        }
      }
      else {
        uVar8 = 0;
        uVar2 = FUN_005a7400(0);
        uVar3 = FUN_0043bc20(0xffffffff,0);
        puVar7 = local_18;
        FUN_0043cb00(puVar7,uVar3,uVar2);
        puVar4 = (undefined4 *)FUN_004014c0(puVar7,uVar3);
        iVar5 = FUN_00445ea0(*puVar4,puVar4[1],uVar2,uVar8);
        if (iVar5 == 0) {
          param_1[0x16] = (int)((float)param_1[0x16] - (float)_DAT_00697ce8);
        }
      }
    }
  }
  return;
}

