// FUN_00446ca0 @ 0x446ca0

void FUN_00446ca0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = operator_new(8);
  if (puVar1 == (undefined4 *)0x0) {
    puVar1 = (undefined4 *)0x0;
  }
  else {
    *puVar1 = 0;
    puVar1[1] = 0;
  }
  puVar1[1] = param_2;
  *puVar1 = param_3;
  puVar2 = (undefined4 *)FUN_00446890();
  *puVar2 = puVar1;
  return;
}

