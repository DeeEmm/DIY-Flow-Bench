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
using DiyFlowBench.Api.Responses;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace DiyFlowBench.Api.Test.Responses
{
    [TestClass]
    public class GetVersionResponseTests
    {
        [TestMethod]
        public void GetVersionResponse_WhenConstructed_BuildNumberIsParsedCorrectly()
        {
            //Arrange
            GetVersionCommand command = new GetVersionCommand();
            byte[] data = ResponseHelper.AppendChecksum(new byte[] { 0x56, 0x01, 0x02, 0x03 });
            byte[] test = ResponseHelper.AppendChecksum(new byte[] { 0x56, 0x78, 0x9A, 0xBC });

            //Act
            GetVersionResponse response = new GetVersionResponse(command, data);

            //Assert
            Assert.AreEqual(0x01, response.Major, "The Major version number is not correct.");
            Assert.AreEqual(0x02, response.Minor, "The Minor version number is not correct.");
            Assert.AreEqual(0x03, response.Revision, "The Revision version number is not correct.");
        }
    }
}
