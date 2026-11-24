// FUN_00420450 @ 0x420450

undefined4 FUN_00420450(int *param_1,int param_2,undefined4 *param_3)

{
  int *piVar1;
  undefined1 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  
  if (param_2 == 0) {
    iVar3 = FUN_00420380(param_1,0,param_3);
    if (iVar3 == 3) {
      FUN_005b51f0(param_1);
    }
    else if (iVar3 == 0) {
      uVar2 = (**(code **)(*param_1 + 0x54))();
      FUN_005b6520(param_1,uVar2);
    }
    else if (iVar3 == 4) {
      uVar8 = 0;
      puVar4 = (undefined4 *)FUN_0043bc20(0,0);
      uVar5 = puVar4[1];
      uVar7 = *puVar4;
      uVar6 = 0x18;
      puVar4 = (undefined4 *)FUN_00401930(0x18,uVar7,uVar5,uVar8);
      FUN_005629b0(*puVar4,puVar4[1],uVar6,uVar7,uVar5,uVar8);
      FUN_004047a0(1);
    }
    else if (iVar3 == 2) {
      *param_3 = 0;
      piVar1 = (int *)param_3[1];
      if (piVar1 != (int *)0x0) {
        FUN_0043c800();
        iVar3 = FUN_006253a0();
        if (iVar3 == 1) {
          (**(code **)(*param_1 + 0xc))(3,piVar1);
          (**(code **)(*piVar1 + 0xc))(0x1a,0);
        }
      }
    }
    uVar5 = 1;
  }
  else {
    uVar5 = 0;
  }
  return uVar5;
}

