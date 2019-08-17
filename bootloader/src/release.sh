# $Id$

configs=( MBHP_CORE_STM32 )

for i in "${configs[@]}"; do
  echo "Building for $i"
  source ../../source_me_${i}
  make cleanall
  make MBHP_CORE_STM32
done

 make cleanall

###############################################################################
echo "Done!"
