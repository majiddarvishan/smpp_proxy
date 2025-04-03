find ../ -iname *.h | xargs clang-format-16 --dry-run --Werror
find ../ -iname *.cpp | xargs clang-format-16 --dry-run --Werror
