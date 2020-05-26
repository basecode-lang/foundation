import lldb
import lldb.formatters
import re
import os


def log(fmt, *args, **kwargs):
    logger = lldb.formatters.Logger.Logger()
    logger >> fmt.format(*args, **kwargs)


def extract_template_arg(val, i):
    data_type = val.GetType().GetUnqualifiedType()
    if data_type.IsReferenceType():
        data_type = data_type.GetDereferenedType()
    if data_type.GetNumberOfTemplateArguments() > i:
        data_type = data_type.GetTemplateArgumentType(i)
    else:
        data_type = None
    return data_type


class Stack_Provider(lldb.SBSyntheticValueProvider):
    def __init__(self, valobj, internal_dict):
        self.val = valobj
        self.data = None
        self.size = None
        self.data_type = None

    def update(self):
        try:
            self.data_type = extract_template_arg(self.val, 0)
            self.data = self.val.GetChildMemberWithName('data')
            self.size = self.val.GetChildMemberWithName('size').GetValueAsUnsigned()
        except:
            pass

    def num_children(self):
        return self.size

    def has_children(self):
        return self.size > 0

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
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
        self.size = None
        try:
            self.data_type = extract_template_arg(self.val, 0)
            self.data = self.val.GetChildMemberWithName('data')
            self.size = self.val.GetChildMemberWithName('size').GetValueAsUnsigned()
        except:
            pass

    def num_children(self):
        return self.size

    def has_children(self):
        return self.size > 0

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def get_child_at_index(self, index):
        index_name = '[{}]'.format(index)
        offset = index * self.data_type.GetByteSize()
        return self.data.CreateChildAtOffset(index_name, offset, self.data_type)


def __lldb_init_module(debugger, dict):
    log("in __lldb_init_module")
    debugger.HandleCommand("type synthetic add -x \"basecode::array_t<.+>\" --python-class basecode.Array_Provider")
    debugger.HandleCommand("type synthetic add -x \"basecode::stack_t<.+>\" --python-class basecode.Stack_Provider")
    debugger.HandleCommand("type summary add -e -h --summary-string "
                           "\"alloc=${var.alloc} capacity=${var.capacity} size=${svar%#}\" "
                           " -w basecode -x \"basecode::stack_t<.+>\"")
    debugger.HandleCommand("type summary add -e -h --summary-string "
                           "\"alloc=${var.alloc} capacity=${var.capacity} size=${svar%#}\" "
                           " -w basecode -x \"basecode::array_t<.+>\"")
    debugger.HandleCommand("type category enable basecode")
