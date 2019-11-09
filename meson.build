project('charlie', 'cpp',
        version : '0.1',
        default_options : ['warning_level=3', 'cpp_std=c++17'])

llvm_dep = dependency('llvm')

srcs = [
   'src/main.cpp',
   'src/parser.cpp',
   'src/ast.cpp',
   'src/lexer.cpp',
]

deps = [
    llvm_dep
]

executable('charlie',
           sources: srcs,
           dependencies: deps,
           install : true)
