import json
import shutil

from op_base import OperationBase
from context import Context
from ameba_enums import *
from utility import *

class Helper(OperationBase):
    cmd_help_msg = 'Common helpers'

    def __init__(self, context:Context) -> None:
        super().__init__(context)

    @staticmethod
    def register_args(parser) -> None:
        subparsers = parser.add_subparsers(dest='sub_operation', help='Available operations for helper', required=True)

        #NOTE: args for manifest-fmt
        sub = subparsers.add_parser('manifest-fmt', help='Format manifest file from json5 to json format')
        sub.add_argument('-o', '--output-file', help='Output formatted file', required=True)

        #NOTE: args for merge
        sub = subparsers.add_parser('merge', help='Merge files to single one')
        sub.add_argument('-o', '--output-file', help='Output file', required=True)
        sub.add_argument('-i', '--input-file', nargs='+', help='Input files', required=True)

    @staticmethod
    def require_manifest_file(context:Context) -> bool:
        if context.args.sub_operation == "manifest-fmt":
            return True
        else:
            return False

    @staticmethod
    def require_layout_file(context:Context) -> bool:
        return False

    def pre_process(self) -> Error:
        if self.context.args.sub_operation == "merge":
            for f in self.context.args.input_file:
                if not os.path.exists(f):
                    return Error(ErrorType.FILE_NOT_FOUND, f)
        return Error.success()

    def process(self) -> Error:
        if self.context.args.sub_operation == "manifest-fmt":
            return self.manifest_format(
                self.context.args.output_file,
            )
        elif self.context.args.sub_operation == "merge":
            return self.merge(
                self.context.args.output_file,
                *self.context.args.input_file
            )
        else:
            return Error(ErrorType.INVALID_INPUT)

    def post_process(self) -> Error:
        return Error.success()

    @exit_on_failure(catch_exception=True)
    def manifest_format(self, output_file:str) -> Error:
        with open(output_file, "w") as f:
            json.dump(manifest_preprocess(self.context.manifest_data), f, indent=2)
        return Error.success()

    @exit_on_failure(catch_exception=True)
    def merge(self, output_file:str, *input_file) -> Error:
        with open(output_file, "wb") as out_f:
            for i in input_file:
                with open(i, 'rb') as in_f:
                    shutil.copyfileobj(in_f, out_f)
        return Error.success()
