<?
  # Change this path to where you're putting your users .html files.

  $dirpath="/home/squirt/webpage/dcove/users";

  # The whofile, this needs to be defined properly, because well if
  # it isn't then the whofile will be generated as a normal userfile
  # when going the the user directory.

  $whofile="whofile.html";

  # DO NOT MODIFY BEYOND THIS POINT UNLESS YOU KNOW WHAT YOU ARE
  # DOING! The only files you should have to edit are the 'header.php'
  # and 'footer.php' found in the same directory as this one.
  if ($action=='')
  {
	include("header.php");
?>
<center>
<table width="80%" border=3 cellspacing=1 cellpadding=4>

<tr>
<td align="left" valign="center" colspan="3">
<font size="+2">To see who's currently online, click <a href="whofile.html">here</a>,</font></td>
</tr>

<tr>
<form method="post" action="index.php?action=sort">
<td align="left" valign="center">
<font size="+1"><b>Or, Sort the users alphabetically:</b></font>
</td>
<td align="left" valign="center">
<font size="+1"><b>
<select name="sortby">
<option value="atoc">[A - C]</option>
<option value="dtof">[D - F]</option>
<option value="gtoi">[G - I]</option>
<option value="jtol">[J - L]</option>
<option value="mtop">[M - P]</option>
<option value="qtos">[Q - S]</option>
<option value="ttov">[T - V]</option>
<option value="wtoz">[W - Z]</option>
</select>
</b></font>
</td>
<td align="center" valign="top">
<font size="+1"><b><input type="submit" value="Sort"></b></font>
</td>
</form>
</tr>

<tr>
<form method="post" action="index.php?action=getuser">
<td align="left" valign="center">
<font size="+1"><b>Or, Enter the user's name:</b></font>
</td>
<td align="left" valign="center">
<font size="+1"><input type="text" name=uname value="" size=15></font>
</td>
<td align="center" valign="top">
<font size="+1"><b><input type="submit" value="Go"></b></font>
</td>
</form>
</tr>

</table>
</center>
<?
	include("footer.php3");
}
else if ($action == "getuser")
{
	strtolower($uname);
	$fname = "$dirpath/$uname.html";
	if ($fd = (@fopen($fname,'r')) == NULL)
	{
		include("header.php");
		echo "<font size=\"+2\">There was no such user ($uname) found... (<a href=\"index.php\">Back</a>)";
		include("footer.php");
		exit;
	}
	close($fd);
	header("Location: $uname.html");
}
else if ($action == "sort")
{
	if ($sortby=="atoc") $bigname="<font size=\"+2\"><b>[A - C]</b></font>";
	else if ($sortby==dtof) $bigname="<font size=\"+2\"><b>[D - F]</b></font>";
	else if ($sortby==gtoi) $bigname="<font size=\"+2\"><b>[G - I]</b></font>";
	else if ($sortby==jtol) $bigname="<font size=\"+2\"><b>[J - L]</b></font>";
	else if ($sortby==mtop) $bigname="<font size=\"+2\"><b>[M - P]</b></font>";
	else if ($sortby==qtos) $bigname="<font size=\"+2\"><b>[Q - S]</b></font>";
	else if ($sortby==ttov) $bigname="<font size=\"+2\"><b>[T - V]</b></font>";
	else if ($sortby==wtoz) $bigname="<font size=\"+2\"><b>[W - Z]</b></font>";
	include("header.php");
	echo "<center>\n<table width=\"80%\" cellpadding=4 cellspacing=1 border=3>\n<tr>\n";
	echo "<td colspan=\"6\" align=\"center\"><font size=\"+1\">Click on the user's name to view their stats.<br>$bigname</center></td>\n</tr>\n<tr>\n";
	$udir = opendir($dirpath);
	$cnt=0;
	while ($html_name = readdir($udir))
	{
		if ($html_name == "." || $html_name=="..") continue;
		if (!strstr($html_name,".html")) continue;
		if ($html_name == $whofile) continue;
		$myname = str_replace(".html","",$html_name);
		if (($sortby==atoc && ($myname[0]=='a' || $myname[0]=='b' || $myname[0]=='c'))
		  || ($sortby==dtof && ($myname[0]=='d' || $myname[0]=='e' || $myname[0]=='f'))
		  || ($sortby==gtoi && ($myname[0]=='g' || $myname[0]=='h' || $myname[0]=='i'))
		  || ($sortby==jtol && ($myname[0]=='j' || $myname[0]=='k' || $myname[0]=='l'))
		  || ($sortby==mtop && ($myname[0]=='m' || $myname[0]=='n' || $myname[0]=='o' || $myname[0]=='p'))
		  || ($sortby==qtos && ($myname[0]=='q' || $myname[0]=='r' || $myname[0]=='s'))
		  || ($sortby==ttov && ($myname[0]=='t' || $myname[0]=='u' || $myname[0]=='v'))
		  || ($sortby==wtoz && ($myname[0]=='w' || $myname[0]=='x' || $myname[0]=='y' || $myname[0]=='z'))) {
			if ($cnt %6 == 0) echo "</tr>\n";
			$cnt += 1;
			echo "<td align=\"center\"><a href=\"$html_name\"><font size=\"+1\"><b>$myname</b></font></a></td>\n";
		}
	}
	closedir($udir);
	if ($cnt%6 != 0)
	{
		if ($cnt%5 == 0) { echo "<td>&nbsp;</td>"; }
		else if ($cnt%4 == 0) { echo "<td>&nbsp;</td>\n<td>&nbsp;</td>"; }
		else if ($cnt%3 == 0) { echo "<td>&nbsp;</td>\n<td>&nbsp;</td>\n<td>&nbsp;</td>"; }
		else if ($cnt%2 == 0) { echo "<td>&nbsp;</td>\n<td>&nbsp;</td>\n<td>&nbsp;</td>\n<td>&nbsp;</td>"; }
		else { echo "<td>&nbsp;</td>\n<td>&nbsp;</td>\n<td>&nbsp;</td>\n<td>&nbsp;</td>\n<td>&nbsp;</td>"; }
	}
	echo "</tr>\n</table>\n</center>\n";
	if ($cnt == 0)
	{
		echo "<center><font size=5>No users were found in this category (<a href=\"index.php\">Back</a>).</font></center>\n";
	}
	else
	{
		$cnt == 1?$tvar="":$tvar="s";
		echo "<center><font size=\"+1\"><b>Total of $cnt user$tvar found. (<a href=\"index.php\">Back</a>)</b></font></center>";
	}
	include("footer.php");
}
?>
