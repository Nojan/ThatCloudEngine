pushd %~dp0

set FROM=D:\Cloud\assets
set TO=%cd%
set ASSIMP=%cd%\..\tools\assimp.exe
set COPY=copy /y


FOR %%G IN (islandvolcano, cityvolcano, islandsrest, island3big, oceanbottom_7, ocean_3, shallowwater5volcano, shallowwater4rest, shallowwater5rest, shallowwater43big, beachvolcano, wavevolcano, wave3big, waverest, treevolcano, beachrest, beach3big, treerest, tree3big) DO (%ASSIMP% export %FROM%\3D\%%G.obj %TO%\3D\%%G.assxml)
rem FOR %%G IN (island_6,beach_2,tree_0,cityvolcano) DO (%ASSIMP% export %FROM%\3D\%%G.obj %TO%\3D\%%G.assxml)

FOR %%G IN (volcano.tga, volcano_beach.tga, city.tga, tree.tga, paw.tga, albino.tga, wet.tga, big.tga, pacman.tga, beach.tga, good.tga, ocean.tga, oceanbottom.tga, shallowwater.tga) DO (%COPY% %FROM%\3D\%%G %TO%\3D\%%G)

FOR /L %%G IN (0,1,59) DO (%COPY% %FROM%\3D\wave_5_%%G.tga %TO%\3D\wave_5_%%G.tga)
