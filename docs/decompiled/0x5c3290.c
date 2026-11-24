// FUN_005c3290 @ 0x5c3290

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005c3290(int *param_1)

{
  float fVar1;
  float fVar2;
  undefined1 uVar3;
  char cVar4;
  int iVar5;
  undefined4 uVar6;
  float10 fVar7;
  undefined1 *puVar8;
  undefined4 uVar9;
  undefined1 local_9c [8];
  undefined1 local_94 [32];
  undefined1 local_74 [16];
  undefined1 local_64 [16];
  undefined1 local_54 [12];
  float local_48;
  float local_44;
  int local_40;
  int local_3c;
  char local_38 [4];
  undefined1 local_34 [16];
  float local_24;
  int local_20;
  int local_1c;
  char local_18 [4];
  undefined1 local_14 [16];
  
  iVar5 = FUN_00507ec0();
  if (iVar5 != 2) {
    if ((char)param_1[0x6c] == '\0') {
      cVar4 = FUN_005498e0();
      if (cVar4 != '\0') {
        param_1[0x6d] = 0;
        FUN_004405c0("Splash",param_1 + 1);
      }
      fVar7 = (float10)FUN_0043e510();
      FUN_00401540(local_9c,(float)fVar7);
      fVar7 = (float10)FUN_0043bc30();
      param_1[0x6e] = (int)(float)(fVar7 + (float10)(float)param_1[0x6e]);
      fVar1 = (float)param_1[0x6e];
      fVar2 = (float)param_1[0x6f];
      if ((NAN(fVar2) || NAN(fVar1)) || fVar2 < fVar1 == (fVar2 == fVar1)) {
        local_20 = param_1[0x18];
        local_1c = param_1[0x19];
        uVar6 = FUN_00401930(0x22000005,0,0,1);
        FUN_005b5220(local_18,param_1,param_1 + 0x16,&local_20,uVar6);
        if (local_18[0] == '\0') {
          FUN_00401510(local_14);
          uVar6 = FUN_0043bc80();
          FUN_00401510(uVar6);
          param_1[0x6d] = 0;
          FUN_004405c0("Splash",param_1 + 1);
        }
        else {
          FUN_00401510(local_14);
        }
      }
      else {
        param_1[0x6d] = _DAT_00698ed8;
        FUN_004405c0("Splash",param_1 + 1);
      }
    }
    else {
      uVar6 = (**(code **)(*param_1 + 4))(&DAT_0067f7e0);
      (**(code **)(*param_1 + 0x10))(uVar6);
      FUN_00462a90();
      uVar3 = FUN_0041f990();
      *(undefined1 *)(param_1 + 0x1a) = uVar3;
      iVar5 = rand();
      local_24 = (float)iVar5 / (float)_DAT_00698f10;
      FUN_00462a90();
      cVar4 = FUN_0041f990();
      if (cVar4 == '\0') {
        fVar1 = (float)_DAT_00698f08 - local_24;
      }
      else {
        fVar1 = local_24 + (float)_DAT_00698dc0;
      }
      uVar6 = FUN_0043bc10(fVar1,local_24 + (float)_DAT_00698f00);
      puVar8 = local_64;
      FUN_00462a90(puVar8,uVar6);
      FUN_00401930();
      uVar6 = FUN_004014c0(puVar8,uVar6);
      FUN_00401510(uVar6);
      FUN_00401510(param_1 + 0x16);
      iVar5 = param_1[0x6d];
      puVar8 = local_74;
      FUN_0043bc80(puVar8,iVar5);
      uVar6 = FUN_00401540(puVar8,iVar5);
      FUN_00401510(uVar6);
      local_40 = param_1[0x18];
      local_3c = param_1[0x19];
      uVar6 = FUN_00401930(0x2000001,0,0,1);
      FUN_005b5220(local_38,param_1,param_1 + 0x16,&local_40,uVar6);
      if (local_38[0] == '\0') {
        FUN_00401510(local_34);
        uVar6 = FUN_0043bc80();
        FUN_00401510(uVar6);
        FUN_0043bc00();
        iVar5 = rand();
        local_44 = (float)iVar5 / (float)_DAT_00698ef8;
        if ((char)param_1[0x1a] == '\0') {
          fVar1 = (float)_DAT_00698ef0 - local_44;
        }
        else {
          fVar1 = local_44 + (float)_DAT_006987c0;
        }
        uVar6 = FUN_0043bc10(fVar1,0.0 - local_44);
        FUN_00401510(uVar6);
        iVar5 = rand();
        local_48 = (float)iVar5 / (float)_DAT_00698ee8;
        uVar9 = 0;
        puVar8 = local_54;
        uVar6 = FUN_0043bc10(0,local_48 + (float)_DAT_00698ee0);
        uVar6 = FUN_004014c0(local_94,uVar6);
        FUN_0061f970(9,uVar6,puVar8,uVar9);
        *(undefined1 *)((int)param_1 + 0xa5) = 1;
      }
      else {
        FUN_00401510(local_34);
        *(undefined1 *)(param_1 + 0x6c) = 0;
      }
    }
  }
  return;
}

