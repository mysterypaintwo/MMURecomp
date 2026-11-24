// FUN_0043c860 @ 0x43c860

uint FUN_0043c860(void)

{
  int iVar1;
  uint in_EAX;
  int *piVar2;
  int unaff_ESI;
  
  iVar1 = *(int *)(in_EAX + 300);
  if (iVar1 != 0) {
    in_EAX = 0;
    if (0 < *(int *)(iVar1 + 0x14)) {
      piVar2 = *(int **)(iVar1 + 0x10);
      do {
        if (unaff_ESI == *piVar2) {
          return CONCAT31((int3)(in_EAX >> 8),1);
        }
        in_EAX = in_EAX + 1;
        piVar2 = piVar2 + 2;
      } while ((int)in_EAX < *(int *)(iVar1 + 0x14));
    }
  }
  return in_EAX & 0xffffff00;
}

