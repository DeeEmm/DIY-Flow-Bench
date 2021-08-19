/***********************************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 ***/

// Development and release version - Don't forget to update the changelog!!
using DiyFlowBench.Api.Commands;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DiyFlowBench.Api.Test.Commands
{
    [TestClass]
    public class CommandTests
    {
        [TestMethod]
        [ExpectedException(typeof(UnsupportedCommandIdentiferException))]
        public void Command_InvalidIdentifier_UnsupportedCommandIdentifierException()
        {
            //Act
            Command command = new GenericCommand(' ');
        }

        [TestMethod]
        public void Command_WhenConstructed_IdentifierIsSaveCorrectly()
        {
            //Act
            Command command = new GenericCommand('V');

            //Assert
            Assert.AreEqual('V', command.Identifier, "Unexpected Identifier.");
        }
    }
}
