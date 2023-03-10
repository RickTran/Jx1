if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[AccCard_History]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)਍ഀ
drop table [dbo].[AccCard_History]਍ഀ
GO਍ഀ
਍ഀ
if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[AccountLog]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)਍ഀ
drop table [dbo].[AccountLog]਍ഀ
GO਍ഀ
਍ഀ
if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Account_Habitus]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)਍ഀ
drop table [dbo].[Account_Habitus]਍ഀ
GO਍ഀ
਍ഀ
if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Account_Info]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)਍ഀ
drop table [dbo].[Account_Info]਍ഀ
GO਍ഀ
਍ഀ
if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Admins]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)਍ഀ
drop table [dbo].[Admins]਍ഀ
GO਍ഀ
਍ഀ
if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[ServerList]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)਍ഀ
drop table [dbo].[ServerList]਍ഀ
GO਍ഀ
਍ഀ
if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[gamerid]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)਍ഀ
drop table [dbo].[gamerid]਍ഀ
GO਍ഀ
਍ഀ
CREATE TABLE [dbo].[AccCard_History] (਍ഀ
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,਍ഀ
	[cAccName] [varchar] (60) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[cCardCode] [varchar] (30) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[dDateTime] [datetime] NULL ,਍ഀ
	[cIP] [char] (15) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[iValue] [int] NULL ,਍ഀ
	[iFlag] [smallint] NULL ਍ഀ
) ON [PRIMARY]਍ഀ
GO਍ഀ
਍ഀ
CREATE TABLE [dbo].[AccountLog] (਍ഀ
	[iID] [bigint] IDENTITY (1, 1) NOT NULL ,਍ഀ
	[szAccName] [varchar] (32) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[iOperation] [bigint] NOT NULL ,਍ഀ
	[szOpAddr] [varchar] (16) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[opData1] [bigint] NULL ,਍ഀ
	[opData2] [varchar] (32) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[optime] [datetime] NOT NULL ਍ഀ
) ON [PRIMARY]਍ഀ
GO਍ഀ
਍ഀ
CREATE TABLE [dbo].[Account_Habitus] (਍ഀ
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,਍ഀ
	[cAccName] [varchar] (60) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[iFlag] [tinyint] NULL ,਍ഀ
	[iLeftSecond] [int] NULL ,਍ഀ
	[dBeginDate] [datetime] NULL ,਍ഀ
	[iLeftMonth] [smallint] NULL ,਍ഀ
	[dEndDate] [datetime] NULL ਍ഀ
) ON [PRIMARY]਍ഀ
GO਍ഀ
਍ഀ
CREATE TABLE [dbo].[Account_Info] (਍ഀ
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,਍ഀ
	[cAccName] [varchar] (32) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[cPassWord] [char] (12) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[cRealName] [varchar] (32) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[dBirthDay] [datetime] NULL ,਍ഀ
	[cArea] [varchar] (60) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[cIDNum] [varchar] (30) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[dRegDate] [datetime] NULL ,਍ഀ
	[cPhone] [varchar] (50) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[iClientID] [bigint] NULL ,਍ഀ
	[dLoginDate] [varchar] (10) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[dLogoutDate] [varchar] (10) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[iTimeCount] [tinyint] NULL ,਍ഀ
	[cQuestion] [varchar] (250) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[cAnswer] [varchar] (250) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[cSex] [varchar] (4) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[cDegree] [varchar] (16) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[cEMail] [varchar] (128) COLLATE Chinese_PRC_CI_AS NULL ਍ഀ
) ON [PRIMARY]਍ഀ
GO਍ഀ
਍ഀ
CREATE TABLE [dbo].[Admins] (਍ഀ
	[szHostAddr] [varchar] (16) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[szUserName] [varchar] (16) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[szPassword] [varchar] (16) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[ePriority] [tinyint] NOT NULL ,਍ഀ
	[eLoggedin] [tinyint] NOT NULL ,਍ഀ
	[dLastLoginTime] [datetime] NULL ,਍ഀ
	[dLastLogoutTime] [datetime] NULL ਍ഀ
) ON [PRIMARY]਍ഀ
GO਍ഀ
਍ഀ
CREATE TABLE [dbo].[ServerList] (਍ഀ
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,਍ഀ
	[cServerName] [varchar] (30) COLLATE Chinese_PRC_CI_AS NOT NULL ,਍ഀ
	[cPassword] [varchar] (12) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[cIP] [varchar] (16) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[iPort] [smallint] NULL ,਍ഀ
	[cMemo] [varchar] (200) COLLATE Chinese_PRC_CI_AS NULL ,਍ഀ
	[dwGamers] [bigint] NULL ਍ഀ
) ON [PRIMARY]਍ഀ
GO਍ഀ
਍ഀ
CREATE TABLE [dbo].[gamerid] (਍ഀ
	[iID] [bigint] IDENTITY (1, 1) NOT NULL ,਍ഀ
	[szName] [varchar] (32) COLLATE Chinese_PRC_CI_AS NOT NULL ਍ഀ
) ON [PRIMARY]਍ഀ
GO਍ഀ
਍ഀ
ALTER TABLE [dbo].[AccCard_History] ADD ਍ഀ
	CONSTRAINT [PK_ACCCARD_HISTORY] PRIMARY KEY  CLUSTERED ਍ഀ
	(਍ഀ
		[iid]਍ഀ
	)  ON [PRIMARY] ਍ഀ
GO਍ഀ
਍ഀ
ALTER TABLE [dbo].[AccountLog] ADD ਍ഀ
	CONSTRAINT [PK_ACCOUNTLOG] PRIMARY KEY  CLUSTERED ਍ഀ
	(਍ഀ
		[szAccName]਍ഀ
	)  ON [PRIMARY] ਍ഀ
GO਍ഀ
਍ഀ
ALTER TABLE [dbo].[Account_Habitus] ADD ਍ഀ
	CONSTRAINT [DF__Account_H__iFlag__7A9C383C] DEFAULT (0) FOR [iFlag],਍ഀ
	CONSTRAINT [PK_ACCOUNT_HABITUS] PRIMARY KEY  CLUSTERED ਍ഀ
	(਍ഀ
		[iid]਍ഀ
	)  ON [PRIMARY] ਍ഀ
GO਍ഀ
਍ഀ
ALTER TABLE [dbo].[Account_Info] ADD ਍ഀ
	CONSTRAINT [DF__Account_I__iGame__77BFCB91] DEFAULT (0) FOR [iClientID],਍ഀ
	CONSTRAINT [PK_Account_Info] PRIMARY KEY  CLUSTERED ਍ഀ
	(਍ഀ
		[iid]਍ഀ
	)  ON [PRIMARY] ਍ഀ
GO਍ഀ
਍ഀ
ALTER TABLE [dbo].[Admins] ADD ਍ഀ
	CONSTRAINT [DF__Admins__ePriorit__7D78A4E7] DEFAULT (0) FOR [ePriority],਍ഀ
	CONSTRAINT [DF__Admins__eLoggedi__7E6CC920] DEFAULT (0) FOR [eLoggedin],਍ഀ
	CONSTRAINT [PK_ADMINS] PRIMARY KEY  CLUSTERED ਍ഀ
	(਍ഀ
		[szUserName]਍ഀ
	)  ON [PRIMARY] ਍ഀ
GO਍ഀ
਍ഀ
ALTER TABLE [dbo].[ServerList] ADD ਍ഀ
	CONSTRAINT [DF__ServerLis__dwGam__014935CB] DEFAULT (0) FOR [dwGamers],਍ഀ
	CONSTRAINT [PK_SERVERLIST] PRIMARY KEY  CLUSTERED ਍ഀ
	(਍ഀ
		[iid]਍ഀ
	)  ON [PRIMARY] ,਍ഀ
	CONSTRAINT [CKC_DWGAMERS_SERVERLI] CHECK ([dwGamers] is null or [dwGamers] >= 0)਍ഀ
GO਍ഀ
਍ഀ
ALTER TABLE [dbo].[gamerid] ADD ਍ഀ
	CONSTRAINT [PK_GAMERID] PRIMARY KEY  CLUSTERED ਍ഀ
	(਍ഀ
		[szName]਍ഀ
	)  ON [PRIMARY] ਍ഀ
GO਍ഀ
਍ഀ
