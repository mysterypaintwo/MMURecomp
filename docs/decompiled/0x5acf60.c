// FUN_005acf60 @ 0x5acf60

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005acf60(int param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  undefined4 uVar4;
  float10 fVar5;
  undefined4 uVar6;
  float local_3c;
  undefined4 local_28;
  float local_24;
  int local_20;
  undefined4 local_1c;
  undefined1 local_18 [4];
  undefined1 local_14 [12];
  int local_8;
  
  if (*(char *)(param_1 + 0x1c5) == '\0') {
    uVar4 = FUN_00401930(0x100200,0,0,1);
    FUN_005b5220(local_18,param_1,param_1 + 0x58,param_1 + 0x60,uVar4);
    FUN_00401510(local_14);
    if (local_8 == 0) {
      if (*(char *)(param_1 + 500) == '\0') {
        FUN_0043e510();
        local_1c = FUN_006253a0();
        local_20 = FUN_0043d3a0(0x2000000,3,local_1c);
        if (local_20 < 0xe0) {
          iVar3 = FUN_00401930();
          if ((float)_DAT_00698818 < *(float *)(param_1 + 0x5c) - *(float *)(iVar3 + 4)) {
            local_28 = *(undefined4 *)(param_1 + 0x58);
            local_24 = (float)local_20 + *(float *)(param_1 + 0x5c);
            uVar6 = 0;
            uVar4 = FUN_0043bc20(0,0);
            FUN_0061f970(6,&local_28,uVar4,uVar6);
            FUN_004026b0("WaterSplash",0,0);
          }
          *(undefined1 *)(param_1 + 500) = 1;
        }
      }
    }
    else {
      FUN_00420a10("TeleportIn");
      FUN_004405c0("Spawn",param_1 + 4);
    }
  }
  else {
    if (*(char *)(param_1 + 0x1c5) == '\0') {
      fVar5 = (float10)FUN_00420130("Fly_Speed");
    }
    else {
      fVar5 = (float10)FUN_0043c800();
      fVar5 = -fVar5;
    }
    local_3c = (float)fVar5;
    fVar5 = (float10)FUN_0043e510();
    *(float *)(param_1 + 0x5c) =
         (float)(fVar5 * (float10)local_3c + (float10)*(float *)(param_1 + 0x5c));
    iVar3 = FUN_0043cb00();
    fVar1 = *(float *)(iVar3 + 4);
    fVar2 = (float)_DAT_00697ce0;
    if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
      fVar5 = (float10)FUN_0043e510();
      *(float *)(param_1 + 0x20c) = (float)((float10)*(float *)(param_1 + 0x20c) - fVar5);
      fVar1 = *(float *)(param_1 + 0x20c);
      fVar2 = (float)_DAT_00697ce0;
      if ((!NAN(fVar1) && !NAN(fVar2)) && fVar1 < fVar2 != (fVar1 == fVar2)) {
        FUN_00507e90(0x40);
        FUN_00441b70(0);
      }
    }
  }
  return;
}

