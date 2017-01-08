/* Fighting constants */
#define CHALLENGE_LINE   "\07~FY Uh Oh...you have been challenged to fight!\n"
#define CHALLENGE_LINE2  "~FT Respond to the fight with~CB: ~CW'~FR.fight~FM yes~CW/~FMno~CW'\n"
#define CHALLENGE_ISSUED "~FB-~CB=~FT[ ~FGYou have issued the challenge..wait and see what happens. ~FT]~CB=~FB-\n"

int num_tie1_text	= 10;
int num_tie2_text	= 10;
int num_wins_text	= 15;
int num_back_out_text	= 10;
int num_challenge_text	= 14;

char tie1_text[][ARR_SIZE] =
{
	"*", /* should never ever be displayed */
	"~FG%s~FM and ~FG%s~FM swing wildly at each other...tiring themselves out..noone wins.. \n",
	"~FMA bunch of people hold back ~FG%s~FM and ~FG%s~FM until they decide not to fight anymore!\n",
	"~FG%s~FM and~FG %s ~FMshake each others hands and laugh at the crowd of onlookers!\n",
	"~FG%s~FM goes to punch ~FG%s~FM and they high five instead...aww no fight :(\n",
	"~FMA police officer comes and arrests ~FG%s~FM and ~FG%s~FM so they cannot fight!\n",
	"~FMA police officer comes and both ~FG%s ~FMand ~FG%s~FM gang up on the cop!\n",
	"~FG%s~FM starts playing with some marbles and ~FG%s ~FMstarts playing too, no fight today!\n",
	"~FG%s ~FMand ~FG%s~FM starts playing with themselves, and get so wrapped up with that, they forget to fight!\n",
	"~FG%s ~FMand ~FG%s~FM realize they are to young to fight, so they stop before they get in trouble!\n",
	"~FG%s ~FMstarts dancing and ~FG%s~FM decides to join in...looks like the fight was cancelled :(\n"
};

char tie2_text[][ARR_SIZE] =
{
	"*", /* should never ever be displayed */
	"~FG%s ~FMand ~FG%s~FM hit each other at the same time, knocking one another out..\n",
	"~FG%s~FM clobbers ~FG%s~FM really good, but then passes out from guilt!\n",
	"~FMA whole crowd of people watch ~FG%s~FM kill ~FG%s~FM and then they kill them!\n",
	"~FG%s~FM watches ~FG%s~FM die....then has a heart attack and dies themself!\n",
	"~FMThey both just up and died....how sad...and wierd!\n",
	"~FG%s~FM and ~FG%s~FM are mistaken as opposing gang members by a rival gang, and are shot on site!\n",
	"~FG%s ~FMtrips, falls and lands on ~FG%s~FM and they both die!!!\n",
	"~FMA mutual enemy of ~FG%s~FM and ~FG%s~FM kills them both and walks into the shadows!\n",
	"~FG%s ~FMgoes postal and shoots ~FG%s~FM and then claims their own life!\n",
	"~FG%s~FM dies and ~FG%s~FM being such a follower decides to die as well!\n"
};

char wins2_text[][ARR_SIZE] =
{
	"*", /* should never ever be displayed */
	"~FY%s~FR sucker punches ~FY%s~FR and they just keel over and die!!!\n",
	"~FY%s ~FRcircles around ~FY%s~FR and then pulls out a gun and shoots them in the head!\n",
	"~FY%s ~FRgoes to kill ~FY%s~FR but a bolt of lightning does it instead!\n",
	"~FY%s ~FRdoes a kung fu move on ~FY%s~FR killing them instantly!\n",
	"~FY%s ~FRstands there while the cops start shooting ~FY%s~FR for no apparent reason!\n",
	"~FRThe cops come and leave ~FY%s~FR alone, but throw ~FY%s~FR in the same cell as Bubba!\n",
	"~FY%s ~FRsneaks into ~FY%s's~FR house irl, and unplugs their phone cord!!\n",
	"~FY%s~FR breathes on ~FY%s~FR and they die from the horrid breath!!\n",
	"~FY%s~FR starts shooting at ~FY%s~FR filling their body with lead!\n",
	"~FRA bunch of ~FY%s's~FR friends gang up on ~FY%s~FR and beat them to a pulp!\n",
	"~FY%s ~FRstarts crying and when ~FY%s~FR goes to see whats wrong they get stabbed!\n",
	"~FY%s~FR sneaks a grenade into ~FY%s's ~FRpants...and next thing you know..~CRKAPOW!!!\n",
	"~FY%s~FR grabs a hockey stick and beats ~FY%s~FR mercilessly with it!\n",
	"~FY%s~FR starts telling jokes about americans,~FY %s~FR gets so insulted they leave..\n",
	"~FY%s~FR passes a beer to ~FY%s~FR, they drink it, and die!\n"
};

char wins1_text[][ARR_SIZE] =
{
	"*", /* should never ever be displayed */
	"~FY%s~FR gives ~FY%s~FR an american beer, and they choke on it and die!\n",
	"~FY%s~FR forces ~FY%s~FR to stay up all night and watch Barney and Friends!\n",
	"~FY%s~FR watches a runaway steamroller run over ~FY%s~FR and chuckles to themself!\n",
	"~FY%s~FR calls the ambulance for ~FY%s~FR before the fight..it gets there, and runs over them...!\n",
	"~FY%s~FR summons an unearthly force...next thing you know ~FY%s~FR is gone!\n",
	"~FRA falling anvil falls...missing ~FY%s~FR but flattening ~FY%s~FR!!!\n",
	"~FY%s~FR smokes a joint with ~FY%s~FR they can't handle it and die!\n",
	"~FY%s~FR throws a ball at ~FY%s~FR, they catch it, and it blows them to pieces!\n",
	"~FY%s~FR hands ~FY%s~FR some canadian beer, and they pass out unable to handle the stuff!\n",
	"~FY%s~FR calls the cops and runs away...they come and arrest ~FY%s~FR!\n",
	"~FY%s~FR watches the earth devour ~FY%s~FR and then just close up!\n",
	"~FY%s~FR pours concrete on ~FY%s's~FR shoes and shoves them in a river!!\n",
	"~FY%s~FR grinds up glass and puts it in ~FY%s's ~FRdrink...ummm that wasn't pretty!\n",
	"~FY%s~FR farts on ~FY%s~FR and knocks them out from the smell..along with everyone else around!\n",
	"~FY%s~FR hands an exploding cigar to ~FY%s~FR and watches their head get blown off!\n"
};

char back_out_text[][ARR_SIZE] =
{
	"*", /* should never ever be displayed */
	"~CT%s's~CG mommy is calling, and has to go and clean up...\n",
	"~CT%s ~CGis scared and hides under some bushes!\n",
	"~CT%s~CG ran away and called the police...who came and broke up the fight!\n",
	"~CT%s's~CG friends came to help out...but they all cowered away!\n",
	"~CT%s ~CGdrops on their knees and pleads for their life and for forgiveness!\n",
	"~CT%s ~CGstarts crying like a lil baby, and begs for forgiveness...\n",
	"~CT%s ~CGlooks around for Squirt, but doesn't find him so they run away from the fight!\n",
	"~CT%s ~CGsays ~CW\"~FRMy mom doesn't believe in fighting!~CW\"~CG, and just walks away!\n",
	"~CT%s ~CGpretends they didn't even get a fight challenge...hoping noone else noticed!\n",
	"~CT%s ~CGsits down and refuses to fight....whattttta WIMP!\n"
};

char challenge_text[][ARR_SIZE] =
{
	"*", /* should never ever be displayed */
	"~FT%s~FY starts slapping ~FT%s~FY like the bitch they are...looks like a fight!\n",
	"~FT%s~FY crouches behind ~FT%s~FY tripping them..laughing as they hit the ground!\n",
	"~FT%s~FY approaches ~FT%s~FY and slaps them across the face...\n",
	"~FT%s~FY slaps ~FT%s~FY upside the head....looks like a fight!\n",
	"~FT%s~FY starts screaming in ~FT%s's~FY face, wanting to start a fight!\n",
	"~FT%s~FY spits on ~FT%s's~FY shoes and says ~CW\"~FGI spit on your feet!~CW\"\n",
	"~FT%s~FY sneezes on ~FT%s~FY intentionally covering them with a bunch of boogers!\n",
	"~FT%s~FY starts making momma jokes about ~FT%s~FY trying to goad them into a fight!\n",
	"~FT%s~FY goes up and steals ~FT%s's~FY hat...uh oh...someones looking for a fight!\n",
	"~FT%s~FY sneaks up behind ~FT%s~FY and shoves them to the ground...\n",
	"~FT%s~FY sneaks up behind ~FT%s~FY and elbows them in the back of the head...\n",
	"~FT%s~FY screams in ~FT%s's~FY ear about how annoying and ugly they are...\n",
	"~FT%s~FY rips ~FT%s's~FY pants off and starts laffing at them.....\n",
	"~FT%s~FY shouts out~CW \"~FT%s~FR you piece of sh*t, prepare to die!!~CW\"\n"
};

char *hanged[8] =
{
	"~CR .------.\n~CR        | ~FTHangman word~CB:   ~FG%s\n~CR        | ~FTLetters guessed~CB:   ~FG%s\n~CR        |\n~CR  ------'\n",
	"~CR .------.\n~CY ~ ~CR     | ~FTHangman word~CB:   ~FG%s\n~CR        | ~FTLetters guessed~CB:   ~FG%s\n~CR        |\n~CR  ------'\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CR        | ~FTLetters guessed~CB:   ~FG%s\n~CR        |\n~CR  ------'\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CY |   ~CR   | ~FTLetters guessed~CB:   ~FG%s\n~CR        |\n~CR  ------'\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CY\\|  ~CR    | ~FTLetters guessed~CB:   ~FG%s\n~CR        |\n~CR  ------'\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CY\\|/ ~CR    | ~FTLetters guessed~CB:   ~FG%s\n~CR        |\n~CR  ------'\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CY\\|/ ~CR    | ~FTLetters guessed~CB:   ~FG%s\n~CY/~  ~CR    |\n~CR  ------'\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CY\\|/ ~CR    | ~FTLetters guessed~CB:   ~FG%s\n~CY/~\\  ~CR   |\n~CR  ------'\n"
};

char *tphanged[8] =
{
	"~CR .------.\n~CR        | ~FTHangman word~CB:   ~FG%s\n~CR        | ~FTGuessed~CB:   ~FG%s\n~CR        | ~FT%s ~FGwins~CB:   ~FM%d ~FT%s ~FGwins~CB:    ~FM%d\n~CR  ------' ~FTRounds left~CB:   ~FG%d\n",
	"~CR .------.\n~CY ~~CR      | ~FTHangman word~CB:   ~FG%s\n~CR        | ~FTGuessed~CB:   ~FG%s\n~CR        | ~FT%s ~FGwins~CB:   ~FM%d ~FT%s ~FGwins~CB => ~FM%d.\n~CR  ------' ~FTRounds left ~CB=> ~FG%d\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CR        | ~FTGuessed~CB => ~FG%s\n~CR        | ~FT%s ~FGwins ~CB=>~FM %d ~FT%s ~FGwins ~CB=> ~FM%d.\n~CR  ------' ~FTRounds left ~CB=> ~FG%d\n",
	"~CR .------.\n~CY(~)~CR     | ~FTHangman word~CB:   ~FG%s\n~CY | ~CR     | ~FTGuessed~CB:   ~FG%s\n~CR        | ~FT%s ~FGwins ~CB=> ~FM%d ~FT%s ~FGwins~CB =>~FM %d.\n~CR  ------' ~FTRounds left ~CB=> ~FG%d\n",
	"~CR .------.\n~CY(~) ~CR    | ~FTHangman word~CB:   ~FG%s\n~CY\\| ~CR     | ~FTGuessed~CB => ~FG%s\n~CR        | ~FT%s ~FGwins ~CB=> ~FM%d ~FT%s ~FGwins~CB =>~FM%d.\n~CR  ------' ~FTRounds left ~CB=> ~FG%d\n",
	"~CR .------.\n~CY(~) ~CR    | ~FTHangman word~CB:   ~FG%s\n~CY\\|/ ~CR    | ~FTGuessed~CB => ~FG%s\n~CR        | ~FT%s ~FGwins ~CB=>~FM %d ~FT%s ~FGwins~CB =>~FM %d.\n~CR  ------' ~FTRounds left ~CB=> ~FG%d\n",
	"~CR .------.\n~CY(~) ~CR    | ~FTHangman word~CB:   ~FG%s\n~CY\\|/ ~CR    | ~FTGuessed~CB => ~FG%s\n~CY/~  ~CR    | ~FT%s ~FGwins ~CB=>~FM %d ~FT%s ~FGwins~CB =>~FM %d.\n~CR  ------' ~FTRounds left ~CB=> ~FG%d\n",
	"~CR .------.\n~CY(~) ~CR    | ~FTHangman word~CB:   ~FG%s\n~CY\\|/ ~CR    | ~FTGuessed~CB => ~FG%s\n~CY/~\\ ~CR    | ~FT%s ~FGwins ~CB=> ~FM%d ~FT%s ~FGwins~CB => ~FM%d.\n~CR  ------' ~FTRounds left ~CB=> ~FG%d\n"
};

/* Coins for .toss game */
char *heads_coin = "~CY   .-\"\"\"-.\n~CY .`~CK  ...  ~CY`.\n~CY/ ~CR  o   o   ~CY\\\n~CY;~FY    /      ~CY;\n~CY\\ ~FY  `--     ~CY/\n~CY '.~FR`-----'~CY.'\n~CY   '-----'\n";
char *tails_coin = "~CY   .-\"\"\"-.\n~CY .`       `.\n~CY/~FY   .   .   ~CY\\\n~CY;~FY  (  |  )  ~CY;\n~CY\\ ~FY  `   '   ~CY/\n~CY '.       .'\n~CY   '-----'\n";
