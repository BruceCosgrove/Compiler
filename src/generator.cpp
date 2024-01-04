#include "generator.hpp"

namespace shl
{
    std::string generator::generate()
    {
        generate_return();

        std::string assembly = _output.str();
        _output = {};
        return assembly;
    }

    void generator::generate_return()
    {
        _output << "global _start\n_start:\n";
        _output << indent << "mov rax, 60\n";
        _output << "    mov rdi, " << _root._expression._integer_literal.value.value() << '\n';
        _output << indent << "syscall\n";
    }
} // namespace shl
