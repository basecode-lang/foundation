import lldb
import lldb.formatters


def log(fmt, *args, **kwargs):
    logger = lldb.formatters.Logger.Logger()
    logger >> fmt.format(*args, **kwargs)


def extract_template_arg(valobj, i):
    base_type = valobj.GetType().GetUnqualifiedType()
    if base_type.IsReferenceType():
        base_type = base_type.GetDereferencedType()
    if base_type.GetNumberOfTemplateArguments() > i:
        data_type = base_type.GetTemplateArgumentType(i)
    else:
        data_type = None
    return data_type


class Stack_Provider(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj, dict):
        self.val = valobj
        self.data = None
        self.size = None
        self.data_type = None

    def update(self):
        self.data = self.val.GetChildMemberWithName('data')
        self.size = self.val.GetChildMemberWithName('size').GetValueAsUnsigned()
        self.data_type = extract_template_arg(self.val, 0)
        if self.data_type is None:
            self.data_type = self.data.GetType().GetPointeeType()
        pass

    def num_children(self):
        return self.size

    def has_children(self):
        return self.size > 0

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return None

    def get_child_at_index(self, index):
        if self.data_type is None:
            return None
        index_name = '[{}]'.format(index)
        offset = (self.size - index - 1) * self.data_type.GetByteSize()
        return self.data.CreateChildAtOffset(index_name, offset, self.data_type)


class Array_Provider(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj, internal_dict):
        self.val = valobj
        self.data = None
        self.size = None
        self.data_type = None

    def update(self):
        self.data = self.val.GetChildMemberWithName('data')
        self.size = self.val.GetChildMemberWithName('size').GetValueAsUnsigned()
        self.data_type = extract_template_arg(self.val, 0)
        if self.data_type is None:
            self.data_type = self.data.GetType().GetPointeeType()
        pass

    def num_children(self):
        return self.size

    def has_children(self):
        return self.size > 0

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return None

    def get_child_at_index(self, index):
        if self.data_type is None:
            return None
        index_name = '[{}]'.format(index)
        offset = index * self.data_type.GetByteSize()
        return self.data.CreateChildAtOffset(index_name, offset, self.data_type)


def __lldb_init_module(debugger, dict):
    log("in __lldb_init_module")
    debugger.HandleCommand("type synthetic add -x \"basecode::array_t<.+>\" "
                           "--python-class basecode.Array_Provider")
    debugger.HandleCommand("type synthetic add -x \"basecode::stack_t<.+>\" "
                           "--python-class basecode.Stack_Provider")
    debugger.HandleCommand("type synthetic add -x \"basecode::stable_array_t<.+>\" "
                           "--python-class basecode.Array_Provider")
    debugger.HandleCommand("type summary add -e -h --summary-string "
                           "\"alloc=${var.alloc} capacity=${var.capacity} size=${svar%#}\" "
                           " -w basecode -x \"basecode::stack_t<.+>\"")
    debugger.HandleCommand("type summary add -e -h --summary-string "
                           "\"alloc=${var.alloc} capacity=${var.capacity} size=${svar%#}\" "
                           " -w basecode -x \"basecode::array_t<.+>\"")
    debugger.HandleCommand("type summary add -e -h --summary-string "
                           "\"alloc=${var.alloc} capacity=${var.capacity} size=${svar%#}\" "
                           " -w basecode -x \"basecode::stable_array_t<.+>\"")
    debugger.HandleCommand("type category enable basecode")
