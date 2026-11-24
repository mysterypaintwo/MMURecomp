// FUN_0043e050 @ 0x43e050

undefined4 * __thiscall FUN_0043e050(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar2 = *(undefined4 **)(param_1 + 0x18);
  puVar3 = (undefined4 *)puVar2[1];
  if (*(char *)((int)puVar3 + 0x15) == '\0') {
    do {
      cVar1 = FUN_0042fc40(puVar3 + 3,param_2);
      if (cVar1 == '\0') {
        puVar4 = (undefined4 *)*puVar3;
        puVar2 = puVar3;
      }
      else {
        puVar4 = (undefined4 *)puVar3[2];
      }
      puVar3 = puVar4;
    } while (*(char *)((int)puVar4 + 0x15) == '\0');
  }
  return puVar2;
}

