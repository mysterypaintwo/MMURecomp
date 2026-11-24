// FUN_00430ca0 @ 0x430ca0

CXMLNode<IXMLDOMNode> * __thiscall FUN_00430ca0(CXMLNode<IXMLDOMNode> *param_1,uint param_2)

{
  ATL::CXMLNode<IXMLDOMNode>::~CXMLNode<IXMLDOMNode>(param_1);
  if ((param_2 & 1) != 0) {
    operator_delete(param_1);
  }
  return param_1;
}

