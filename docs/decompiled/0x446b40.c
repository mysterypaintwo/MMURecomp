// FUN_00446b40 @ 0x446b40

void * FUN_00446b40(undefined4 param_1,undefined4 param_2,undefined4 *param_3)

{
  void *pvVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  
  pvVar1 = operator_new(0xc);
  uVar2 = FUN_00430df0(pvVar1);
  puVar3 = (undefined4 *)FUN_00401080(4,uVar2);
  if (puVar3 != (undefined4 *)0x0) {
    *puVar3 = param_1;
  }
  uVar2 = FUN_0042fd70(pvVar1);
  puVar3 = (undefined4 *)FUN_00401080(4,uVar2);
  if (puVar3 != (undefined4 *)0x0) {
    *puVar3 = param_2;
  }
  puVar3 = (undefined4 *)FUN_00401080(4,(int)pvVar1 + 8);
  if (puVar3 != (undefined4 *)0x0) {
    *puVar3 = *param_3;
  }
  return pvVar1;
}

