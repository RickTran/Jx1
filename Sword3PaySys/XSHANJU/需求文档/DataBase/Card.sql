if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[BatchNumber]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[BatchNumber]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[CardCreateRecord]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[CardCreateRecord]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[CardInfo]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[CardInfo]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[CardType]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[CardType]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Card_History]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Card_History]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[MaxNum]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[MaxNum]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[OtherType]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[OtherType]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[PerformIP]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[PerformIP]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[Product]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[Product]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[SaleType]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[SaleType]
GO

if exists (select * from dbo.sysobjects where id = object_id(N'[dbo].[UserManager]') and OBJECTPROPERTY(id, N'IsUserTable') = 1)
drop table [dbo].[UserManager]
GO

CREATE TABLE [dbo].[BatchNumber] (
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,
	[cBatchNumberCode] [varchar] (6) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cBatchNumberCodeDescrip] [varchar] (256) COLLATE Chinese_PRC_CI_AS NULL ,
	[iFlag] [int] NULL ,
	[dDate] [datetime] NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[CardCreateRecord] (
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,
	[cUserID] [varchar] (16) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[binFile] [image] NULL ,
	[dDate] [datetime] NULL ,
	[cProductCode] [varchar] (2) COLLATE Chinese_PRC_CI_AS NULL ,
	[cCardTypeCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NULL ,
	[cBatchNumberCode] [varchar] (6) COLLATE Chinese_PRC_CI_AS NULL ,
	[cBeginCardCode] [varchar] (20) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cEndCardCode] [varchar] (20) COLLATE Chinese_PRC_CI_AS NOT NULL 
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO

CREATE TABLE [dbo].[CardInfo] (
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,
	[cCardCode] [varchar] (30) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cCardPassWord] [varchar] (16) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[iFlag] [bit] NULL ,
	[iHoldSecond] [int] NULL ,
	[iHoldMonth] [tinyint] NULL ,
	[cOverdueDate] [datetime] NULL ,
	[cDate] [datetime] NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[CardType] (
	[iid] [int] IDENTITY (1, 1) NOT NULL ,
	[cTypeCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cTypeDescrip] [varchar] (256) COLLATE Chinese_PRC_CI_AS NULL ,
	[iFlag] [smallint] NULL ,
	[iValue] [int] NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[Card_History] (
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,
	[cCardCode] [varchar] (30) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[dDate] [datetime] NULL ,
	[cUserName] [varchar] (32) COLLATE Chinese_PRC_CI_AS NULL ,
	[iFlag] [smallint] NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[MaxNum] (
	[cProductCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cTypeCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cOtherTypeCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cSaleTypeCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[iNum] [int] NOT NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[OtherType] (
	[iid] [int] IDENTITY (1, 1) NOT NULL ,
	[cOtherTypeCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cOtherTypeDescrip] [varchar] (256) COLLATE Chinese_PRC_CI_AS NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[PerformIP] (
	[iid] [bigint] IDENTITY (1, 1) NOT NULL ,
	[cPerformIP] [varchar] (16) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cMemo] [varchar] (255) COLLATE Chinese_PRC_CI_AS NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[Product] (
	[iid] [int] IDENTITY (1, 1) NOT NULL ,
	[cProductCode] [varchar] (2) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cProductDescrip] [varchar] (256) COLLATE Chinese_PRC_CI_AS NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[SaleType] (
	[iid] [int] IDENTITY (1, 1) NOT NULL ,
	[cSaleTypeCode] [varchar] (4) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cSaleTypeDescrip] [varchar] (256) COLLATE Chinese_PRC_CI_AS NULL 
) ON [PRIMARY]
GO

CREATE TABLE [dbo].[UserManager] (
	[iid] [int] IDENTITY (1, 1) NOT NULL ,
	[cUserCode] [varchar] (20) COLLATE Chinese_PRC_CI_AS NOT NULL ,
	[cUserName] [varchar] (30) COLLATE Chinese_PRC_CI_AS NULL ,
	[iRole] [smallint] NULL ,
	[iFlag] [bit] NULL ,
	[cPassWord] [varchar] (20) COLLATE Chinese_PRC_CI_AS NULL ,
	[cEmail] [varchar] (128) COLLATE Chinese_PRC_CI_AS NULL 
) ON [PRIMARY]
GO

ALTER TABLE [dbo].[BatchNumber] ADD 
	CONSTRAINT [PK_BatchNumber] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[CardCreateRecord] ADD 
	CONSTRAINT [PK_CardCreateRecord] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[CardInfo] ADD 
	CONSTRAINT [PK_CardInfo] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[CardType] ADD 
	CONSTRAINT [PK_CardType] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[Card_History] ADD 
	CONSTRAINT [PK_Card_History] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[MaxNum] ADD 
	CONSTRAINT [PK_MaxNum] PRIMARY KEY  CLUSTERED 
	(
		[cProductCode],
		[cTypeCode],
		[cOtherTypeCode],
		[cSaleTypeCode]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[OtherType] ADD 
	CONSTRAINT [PK_OtherType] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[PerformIP] ADD 
	CONSTRAINT [PK_PerformIP] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[Product] ADD 
	CONSTRAINT [PK_Product] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[SaleType] ADD 
	CONSTRAINT [PK_SaleType] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

ALTER TABLE [dbo].[UserManager] ADD 
	CONSTRAINT [PK_User] PRIMARY KEY  CLUSTERED 
	(
		[iid]
	)  ON [PRIMARY] 
GO

Insert into UserManager(cUserCode,cUserName,iRole,iFlag,cPassWord,cEmail) values('system','system',1,1,'system','test@kingsoft.net')
GO