// FUN_00500000 @ 0x500000

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00500000(int param_1)

{
  float *pfVar1;
  float10 fVar2;
  undefined1 *puVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  undefined1 local_14 [8];
  undefined1 local_c [8];
  
  pfVar1 = (float *)FUN_00431270();
  if ((float)_DAT_00697ce0 <= *pfVar1) {
    FUN_004405c0("SelectLevel",param_1 + 4);
  }
  else {
    uVar4 = 0;
    fVar2 = (float10)FUN_0043e510(0);
    pfVar1 = (float *)FUN_00431270((float)(fVar2 * (float10)_DAT_00698aa0));
    fVar2 = (float10)FUN_004ff020(-*pfVar1);
    FUN_0043bc10((float)fVar2,uVar4);
    puVar5 = local_c;
    puVar3 = local_14;
    FUN_00431270(puVar3,puVar5);
    uVar4 = FUN_004014c0(puVar3,puVar5);
    FUN_004026e0(uVar4);
    puVar5 = local_c;
    puVar3 = local_1c;
    FUN_00431270(puVar3,puVar5);
    uVar4 = FUN_00403d90(puVar3,puVar5);
    FUN_004026e0(uVar4);
    puVar5 = local_c;
    puVar3 = local_24;
    FUN_00431270(puVar3,puVar5);
    uVar4 = FUN_004014c0(puVar3,puVar5);
    FUN_004026e0(uVar4);
  }
  return;
}

