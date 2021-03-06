<?php
require_once('../lib/functions.php3');
if (is_file("../lib/sql/drivers/$config[sql_type]/functions.php3"))
	include_once("../lib/sql/drivers/$config[sql_type]/functions.php3");
else{
	echo "<b>Could not include SQL library</b><br>\n";
	exit();
}
if ($config[sql_use_operators] == 'true'){
	include("../lib/operators.php3");
	$text = ',op';
	$passwd_op = ",':='";
}
$da_abort=0;
$link = @da_sql_pconnect($config);
if ($link){
	$Members = preg_split("/[\n\s]+/",$members,-1,PREG_SPLIT_NO_EMPTY);
	if (!empty($Members)){
		foreach ($Members as $member){
			$res = @da_sql_query($link,$config,
			"INSERT INTO $config[sql_usergroup_table] (UserName,GroupName)
			VALUES ('$member','$login');");
			if (!$res || !@da_sql_affected_rows($link,$res,$config)){
				echo "<b>Unable to add user $member in group $login: " . da_sql_error($link,$config) . "</b><br>\n";
				$da_abort=1;
			}
		}
	}
	if (!$da_abort){
		foreach($show_attrs as $key => $attr){
			if ($attrmap["$key"] == 'none')
				continue;
			if ($attrmap["$key"] == ''){
				$attrmap["$key"] = $key;
				$attr_type["$key"] = 'replyItem';
				$rev_attrmap["$key"] = $key;
			}
			if ($attr_type["$key"] == 'checkItem'){
				$table = "$config[sql_groupcheck_table]";
				$type = 1;
			}
			else if ($attr_type["$key"] == 'replyItem'){
				$table = "$config[sql_groupreply_table]";
				$type = 2;
			}
			$val = $$attrmap["$key"];
			$op_name = $attrmap["$key"] . '_op';
			$op_val = $$op_name;
			if ($op_val != ''){
				if (check_operator($op_val,$type) == -1){
					echo "<b>Invalid operator ($op_val) for attribute $key</b><br>\n";
					coninue;
				}
				$op_val2 = ",'$op_val'";
			}
			if ($val == '' || check_defaults($val,$op_val,$default_vals["$key"]))
				continue;
			$res = @da_sql_query($link,$config,
			"INSERT INTO $table (Attribute,Value,GroupName $text)
			VALUES ('$attrmap[$key]','$val','$login' $op_val2);");
			if (!$res || !@da_sql_affected_rows($link,$res,$config))
				echo "<b>Query failed for attribute $key: " . da_sql_error($link,$config) . "</b><br>\n";
		}
	}
	echo "<b>Group created successfully</b><br>\n";
}
else
	echo "<b>Could not connect to SQL database</b><br>\n";
?>
