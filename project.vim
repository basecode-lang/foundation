" ============================================================================
"
" Project Customization File
"
" ============================================================================
"
function! Project_UpdateTags()
    UpdateTags! -R include/ src/
endfunction

function! Project_Cscope()
    copen 15
    AsyncRun cscope -Rb
endfunction

function! Project_CMake_Generate()
    copen 15
    AsyncRun cd build/clion/clang/debug && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/local/Cellar/llvm/10.0.0_3/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/Cellar/llvm/10.0.0_3/bin/clang++ -G "Ninja" -Wno-dev /Users/jeff/src/basecode-lang/foundation
    " AsyncRun cd build/clion/clang/release && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/local/Cellar/llvm/10.0.0_3/bin/clang -DCMAKE_CXX_COMPILER=/usr/local/Cellar/llvm/10.0.0_3/bin/clang++ -G "Ninja" -Wno-dev /Users/jeff/src/basecode-lang/foundation
endfunction

function! Project_Build_CoreTest(mode)
    copen 15
    if a:mode == 'debug'
        AsyncRun cd build/clion/clang/debug && cmake --build . --target core-test -- -j12
    elseif a:mode == 'release'
        AsyncRun cd build/clion/clang/release && cmake --build . --target core-test -- -j12
    else
        throw "unknown build mode"
    end
endfunction

function! Project_Run_CoreTest(mode)
    copen 15
    if a:mode == 'debug'
        AsyncRun cd build/clion/clang/debug/bin && ./core-test
    elseif a:mode == 'release'
        AsyncRun cd build/clion/clang/release/bin && ./core-test
    else
        throw "unknown run mode"
    end
endfunction

command! PrjCscope              :call Project_Cscope()
command! PrjUpdateTags          :call Project_UpdateTags()
command! PrjCmakeGenerate       :call Project_CMake_Generate()
command! PrjBuildTestDebug      :call Project_Build_CoreTest('debug')
command! PrjBuildTestsRelease   :call Project_Build_CoreTest('release')
command! PrjRunTestsDebug       :call Project_Run_CoreTest('debug')
command! PrjRunTestsRelease     :call Project_Run_CoreTest('release')

