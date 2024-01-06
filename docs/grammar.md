$$
\begin{align}
    \text{[program]} &\to \text{[statement]}^* \\
    \text{[scope]} &\to \{\text{[statement]}^*\} \\
    \text{[statement]} &\to
    \begin{cases}
        \text{[scope]} \\
        return\space\text{[expression]}; \\
        let\space\text{[identifier]}=\text{[expression]}; \\
        if\space\text{[expression]}\text{[scope]} \\
    \end{cases} \\
    \text{[expression]} &\to
    \begin{cases}
        \text{[term]} \\
        \text{[expression]}\text{[binary\_operator]}\text{[expression]} \\
    \end{cases} \\
    \text{[term]} &\to
    \begin{cases}
        \text{[integer\_literal]} \\
        \text{[identifier]} \\
        (\text{[expression]}) \\
    \end{cases} \\
    \text{[binary\_operator]} &\to
    \begin{cases}
        \text{/} & \text{precedence 1} \\
        \text{\%} & \text{precedence 1} \\
        \text{*} & \text{precedence 1} \\
        \text{+} & \text{precedence 0} \\
        \text{-} & \text{precedence 0} \\
    \end{cases} \\
    \text{[integer\_literal]} &\to [0-9]^* \\
    \text{[identifier]} &\to [a-zA-Z][a-zA-Z0-9]^* \\
\end{align}
$$
