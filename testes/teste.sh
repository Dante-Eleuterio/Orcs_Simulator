echo "Testes: ASTAR"
(cd ../OrCS; ./orcs -t ../testes/astar.CINT.PP200M)
echo "Testes CALCULIX"
(cd ../OrCS; ./orcs -t ../testes/calculix.CFP.PP200M)
echo "Testes DEALII"
(cd ../OrCS; ./orcs -t ../testes/dealII.CFP.PP200M)
echo "Testes GROMACS"
(cd ../OrCS; ./orcs -t ../testes/gromacs.CFP.PP200M)
echo "Testes LIBQUANTUM"
(cd ../OrCS; ./orcs -t ../testes/libquantum.CINT.PP200M)
echo "Testes NAMD"
(cd ../OrCS; ./orcs -t ../testes/namd.CFP.PP200M)