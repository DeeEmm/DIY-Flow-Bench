# DIY Flow Bench Project Contribution Guidelines

###

The following guidelines explain how contributions to the project are managed. The reason for having these the guidelines is so that the project is maintained in a well-organised manner and easy to use for both the end user and future developers alike. 
The following guidelines explain how contributions to the project are managed. The reason for having these the guidelines is so that the project is maintained in a well-organised manner and easy to use for both the end user and future developers alike. 

The project stretches across a number of platforms and disciplines, and so trying to make a one-size-fits-all set of 'rules' is likely impossible as there are many things do not apply equally to all. So the guidelines seek to set out some basic concepts that can then be implimented in the best manner across each of the platforms.

## Workflow

There are three basic kinds of branches to the project

- Master Branch - The main default branch asigned by GitHub. This is read only and is at the same level as the current release.
- DEV Branch - This is a staging branch where all code changes are staged. At a new release, changes in DEV are pushed to master.
- Feature branches - Branches for individual features or bug fixes. Once tested feature branches are merged into DEV.

### Basic feature / bug development workflow

- Create an issue for the feature / bugfix
- Request a new issue 'branch' is created (we looking to automate this with git workflows)
- Check out to your local machine by 'Opening with GitHub Desktop'
- Carry out your modifications and changes to your local version
- Be sure to update the changelog and version.json files
- Test your changes!!! Make sure the code compiles and works!!!
- Create a 'pull request' to have your changes merged with the branch
- Your changes will then be reviewed and merged


## Coding standards

As there is no adopted formal coding standard in use, the style and methods employed are largely at the discretion and experience of the individual coder. We beleive that there is no **ONE** _RIGHT_ way of coding (Give a coding job to 10 softies and you will get very different 10 solutions) and so there is no expectation that the code is written a certain way. However with that said, it is expected that all code submissions follow the basic guidelines.


## Workflow

- The 'master' branch contains the current release. Do not branch from this.
- The 'DEV' branch contains the curent code.
- All code changes must be tied to a current issue
- Use the 'create a branch' link on the issue page to create a branch (makes sure you change branch source to 'DEV')
- Checkout the branch to your local machine and make your edits
- Discuss all changes / raise questions in the issue
- When you are done create a Pull Request for your changes.



### Basic guidelines

Whilst we do not employ any specific coding standards, we do however have some very basic guidelines that we ask you to follow if possible.


- One change per PR. Do not combine changes unless they are directly related
- Do not reformat or tidy existing code. Unecessary changes complicate code review
- Try not to make changes after you have created your PR as it complicates the review.
- Keep your work neat so that it is easy for others to follow
- Try to adhere to general good coding practice (no spaghetti logic please)
- Include Doxygen style title blocks for code classes and functions
- Comment your work so that others can follow what's going on (this is a public repo)
- Try to use descriptive variable / function / class names
- Use camelCase for variable / function / class names
- Prefix class references with underscores
- Create constants for integer based logic to make code easier to read (e.g. switch case statements)
- Consider that if you find your code difficult to follow, others will too.
- If in doubt - ASK!


Essentially it's all about making the code easy for others to read and follow. Remember that this is an open source project so people of all skill levels will be using it and trying to decypher what's going on. Also, please consider that the more help that is provided within the code in the form of comments, the less real-world help will be required to get people up and running, which will help minimise the real-world resources needed to support the project.


## Version control

As the project is hosted by GitHub it makes sense to utilise the GIT versioning system. This allows simultaneous colloboration by multiple parties and manages differences between the commits of all parties. 

If you are familiar with GIT or use it as part of your current IDE or Code editor, that's great, then you will already know what to do. However if you have no idea what it is or how to use it then we recommend using GitHub Desktop to manage checking out and committing files. 

How to use GIT is a bit outside of the scope of this document but you can find many how-to's online that explain how to use github desktop. We recommend the user guides that are provided with GitHub Desktop as a good starting point 

If you do get stuck, you can also reach out and give us a shout out on our #developer channel on Discord. If you don't have access, message the owner of the repository or give a shout out on one of the other channels. All contributors are welcomed and encouraged to join the #developer channel.


### GIT Commit Summaries (changelog)

Commit Summaries should use the following format:

>20041501 : Brief Description

Where the number comprises of the current date and a two digit numerical series that increases with every seperate commit on that date:

>YYMMDDXX

NOTE: This is also the same format used in the changelog


### GIT Commit descriptions

Commit descriptions should give a brief list of all changes that are formatted as one change per line. For example:

> : Fixed load issue for Baro and Pitot 

NOTE: This is the same format used in the changelog


## Changelog

In addition to version control we also keep an 'offline' changelog that records the details for each commit. This changelog file resides at /documents/changelog.

The changelog format mirrors that used for the commit summary and description where one change is recorded per line. For example:

>20041501  : Reorganised project folder structure
>
>: Fixed load issue for Baro and Pitot
>
>: Each item occupies its own line for multiple items
>
>20010101  : Fixed spelling issue for ilib library calls

NOTE: The most current entry is at the top of the file

Essentially your workflow should include keeping a track of your changes by updating the changelog file as you are working. This then provides you will all of the info needed when you come to commmit.


## Release Versioning

The software aspects of the project all follow basic release / build versioning where each release has a unique version number that allows identification of individual releases.

Release versioning follows standard **[major].[minor].[build]** numbering Each software change should increment the relevent identifier(s) as follows.

Version numbers are stored in the version.json file in the project root.


### Build version incrementing 

The build number increments every time that the code is updated. Basically every commit that is made should have a different Build number. Currently this is manually managed.

The build number follows the same formatting and is shared with both the commit summaries and changelog where the build number comprises of the current date and a two digit numerical series that increases with every seperate commit on that date:

>YYMMDDXX


### Major / Minor version incrementing

Each official release published on the GutHub releases page should increment the minor release number **unless** there has been a major code refactoring or major feature changes, in which case the Major version number should be incremented and the minor release number reset to zero.

Generally major and minor releases are planned as projects so version number changes are known in advance.


### Development version numbering

Development versioning is used for all major releases. Development releases are identified as a suffix on the version number. The major version number represents that of the new planned release.

The prefixes are as follows:

- A : Alpha release
- B : Beta release
- RC : Release Candidate

For example: V2.0.RC.8

Each stage of development is superceded by the next once any pre-agreed testing or project milestones are completed. Acceptance testing should be performed to allow release candidate testing to be superceded by the actual release.

When moving to the actual release, the RC prefix is dropped.


### Acceptance testing

Prior to the code moving into the Release Candidate stage, acceptance tests should be devised so that the functionality of the software can be tested across a range of scenarios by a range of users. Acceptance tests are usually developed as part of each project as the project grows and may be comprised of a number of smaller unit tests.


### Versioning exposed within software

Version numbering is also made available as variables within the code so that the code version can be displayed to the end user or external apps. For example

>#define MAJOR_VERSION "B1"
>
>#define MINOR_VERSION "2"
>
>#define BUILD_VERSION "20010101"


## Development Workflow

Just as an easy summary of how to best manage workflow. You can of course elect to use your own systems of work.

- Keep the changelog updated with a summary of each change as you work on it
- Update the changelog version with the date that you commit your changes along with the current incremental number
- Update version.json with the relevent build numbers for firmware and / or GUI
- Use the changlog summary and version as your GIT description and summary
