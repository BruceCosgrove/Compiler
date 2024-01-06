$$
\begin{align}
    \text{[program]} &\to \text{[declaration]}^* \\

    \text{[declaration]} &\to
    \begin{cases}
        \text{[definition]} \\
        \text{[variable\_declarations]} \\
        \text{[function\_declaration]} \\
    \end{cases} \\

    \text{[definition]} &\to
    \begin{cases}
        \text{[variable\_definition]} \\
        \text{[function\_definition]} \\
    \end{cases} \\

    \text{[variable\_declarations]} &\to let\space\text{[identifier]}(,let\space\text{[identifier]})? \\

    \text{[variable\_definitions]} &\to let\space\text{[identifier]}=\text{[expression]}(,let\space\text{[identifier]}=\text{[expression]})? \\

    \text{[function\_declaration]} &\to
    \begin{cases}
        function\space\text{[identifier]}(\text{[variable\_declarations]?;?})? \\
        function\space\text{[identifier]}\text{([variable\_declarations]?;[variable\_declarations])} \\
    \end{cases} \\

    \text{[function\_definition]} &\to
    \begin{cases}
        function\space\text{[identifier]}(\text{[variable\_declarations]?;?})?\text{[statement]} \\
        function\space\text{[identifier]}\text{([variable\_declarations]?;[variable\_declarations])}\text{[statement]} \\
    \end{cases} \\

    \text{[alias]} &\to alias\space\text{[identifier]}=\text{[identifier]}\\

    \text{[scope]} &\to \{\text{[scoped\_statement]}^*\} \\

    \text{[scoped\_statement]} &\to
    \begin{cases}
        \text{[statement]} \\
        \text{[declaration]}; \\
        if\space\text{[expression]}\text{[statement]} \\
    \end{cases} \\

    \text{[statement]} &\to
    \begin{cases}
        \text{[scope]} \\
        return\space\text{[expression]}; \\
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
        \text{([expression])} \\
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

    \text{[identifier]} &\to [a-zA-Z\_][a-zA-Z0-9\_]^* \\
\end{align}
$$
