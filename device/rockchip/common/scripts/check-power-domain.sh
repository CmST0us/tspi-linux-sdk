#!/bin/bash -e

dump_kernel_dtb_file=`mktemp`
tmp_phandle_file=`mktemp`
tmp_io_domain_file=`mktemp`
tmp_regulator_microvolt_file=`mktemp`
tmp_final_target=`mktemp`
tmp_grep_file=`mktemp`

dtc -q -I dtb -O dts -o ${dump_kernel_dtb_file} "$RK_KERNEL_DTB"

if [ "$RK_SECURITY_CHECK_METHOD" = "DM-E" ] ; then
	if ! grep -q "compatible = \"linaro,optee-tz\";" $dump_kernel_dtb_file; then
		echo "Please add: "
		echo "        optee: optee {"
		echo "                compatible = \"linaro,optee-tz\";"
		echo "                method = \"smc\";"
		echo "                status = \"okay\";"
		echo "        }"
		echo "To your dts file"
		return 1;
	fi
fi

if ! grep -Pzo "io-domains\s*{(\n|\w|-|;|=|<|>|\"|_|\s|,)*};" $dump_kernel_dtb_file 1>$tmp_grep_file 2>/dev/null; then
	echo "Not Found io-domains in $RK_KERNEL_DTS"
	rm -f $tmp_grep_file
	exit 0
fi
grep -a supply $tmp_grep_file > $tmp_io_domain_file || true
rm -f $tmp_grep_file
awk '{print "phandle = " $3}' $tmp_io_domain_file > $tmp_phandle_file

while IFS= read -r item_phandle && IFS= read -u 3 -r item_domain
do
	echo "${item_domain% *}" >> $tmp_regulator_microvolt_file
	tmp_none_item=${item_domain% *}
	cmds="grep -Pzo \"{(\\n|\w|-|;|=|<|>|\\\"|_|\s)*"$item_phandle\"

	eval "$cmds $dump_kernel_dtb_file | strings | grep "regulator-m..-microvolt" >> $tmp_regulator_microvolt_file" || \
		eval "sed -i \"/${tmp_none_item}/d\" $tmp_regulator_microvolt_file" && continue

	echo >> $tmp_regulator_microvolt_file
done < $tmp_phandle_file 3<$tmp_io_domain_file

while read -r regulator_val
do
	if echo ${regulator_val} | grep supply &>/dev/null; then
		echo -e "\n\n\e[1;33m${regulator_val%*=}\e[0m" >> $tmp_final_target
	else
		tmp_none_item=${regulator_val##*<}
		tmp_none_item=${tmp_none_item%%>*}
		echo -e "${regulator_val%%<*} \e[1;31m$(( $tmp_none_item / 1000 ))mV\e[0m" >> $tmp_final_target
	fi
done < $tmp_regulator_microvolt_file

echo -e "\e[41;1;30m PLEASE CHECK BOARD GPIO POWER DOMAIN CONFIGURATION !!!!!\e[0m"
echo -e "\e[41;1;30m <<< ESPECIALLY Wi-Fi/Flash/Ethernet IO power domain >>> !!!!!\e[0m"
echo -e "\e[41;1;30m Check Node [pmu_io_domains] in the file: $RK_KERNEL_DTS \e[0m"
echo
echo -e "\e[41;1;30m 请再次确认板级的电源域配置！！！！！！\e[0m"
echo -e "\e[41;1;30m <<< 特别是Wi-Fi，FLASH，以太网这几路IO电源的配置 >>> ！！！！！\e[0m"
echo -e "\e[41;1;30m 检查内核文件 $RK_KERNEL_DTS 的节点 [pmu_io_domains] \e[0m"
cat $tmp_final_target
