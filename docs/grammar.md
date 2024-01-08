$$
\begin{align}
    \text{[program]} &\to \text{[declaration]}^* \\

    \text{[declaration]} &\to
    \begin{cases}
        \text{[definition]} \\
        \text{[declare\_variable]}; \\
    \end{cases} \\

    \text{[definition]} &\to
    \begin{cases}
        \text{[define\_variable]}; \\
        \text{[named\_function]}; \\
    \end{cases} \\

    \text{[declare\_variable]} &\to \text{[identifier]}:let \\

    \text{[define\_variable]} &\to \text{[identifier]}:let=\text{[expression]} \\

    \text{[function]} &\to
    \begin{cases}
        :\text{([declare\_variable]}^*\text{;}?\text{)[statement]} \\
        :\text{([declare\_variable]}^*\text{;[parameter]}^+\text{)[statement]} \\
    \end{cases} \\

    \text{[named\_function]} &\to \text{[identifier]}\text{[function]}\\

    \text{[parameter]} &\to \text{[parameter\_pass]}?\space\text{[identifier]}:let \\

    \text{[scope]} &\to \{\text{[scoped\_statement]}^*\} \\

    \text{[statement]} &\to
    \begin{cases}
        \text{[scope]} \\
        return; \\
        \text{[identifier]}=\text{[expression]}; \\
    \end{cases} \\

    \text{[scoped\_statement]} &\to
    \begin{cases}
        \text{[statement]} \\
        \text{[declaration]} \\
        \text{[if]}(elif\space\text{[expression]}\text{[statement]})^*(else\space\text{[expression]}\text{[statement]})? \\
    \end{cases} \\

    \text{[if]} &\to if\space\text{[expression]}\text{[statement]} \\

    \text{[expression]} &\to
    \begin{cases}
        \text{[term]} \\
        \text{[expression]}\text{[operator]}\text{[expression]} \\
    \end{cases} \\

    \text{[term]} &\to
    \begin{cases}
        \text{[integer\_literal]} \\
        \text{[identifier]} \\
        \text{([expression])} \\
    \end{cases} \\

    \text{[operator]} &\to
    \begin{cases}
        \text{/} & \text{precedence 1} \\
        \text{\%} & \text{precedence 1} \\
        \text{*} & \text{precedence 1} \\
        \text{+} & \text{precedence 0} \\
        \text{-} & \text{precedence 0} \\
    \end{cases} \\

    \text{[parameter\_pass]} &\to
    \begin{cases}
        in & \text{default} \\
        out \\
        inout \\
        copy \\
        move \\
    \end{cases} \\

    \text{[integer\_literal]} &\to [0-9]^* \\

    \text{[identifier]} &\to [a-zA-Z\_][a-zA-Z0-9\_]^* \\
\end{align}
$$
