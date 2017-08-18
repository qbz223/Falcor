/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include <random>

namespace Falcor
{
    //	Generator for a Random Float in the range [A, B).
    class RNGFloatRangeInAExB
    {

    public:

        /** Generator for a random number in the half-open range [A, B).
        \param[in] rangeA The lower value of the range.
        \param[in] rangeB The higher value of the range.
        */
        RNGFloatRangeInAExB(float rangeA = 0.0, float rangeB = 1.0)
        {
            std::random_device randomdevice;
            std::seed_seq seed{ randomdevice() };
            mMersenneTwisterGenerator = std::mt19937(seed);

            distribution = std::uniform_real_distribution<float>(rangeA, rangeB);

        };

        /** Generates a new random number.
        */
        float generate()
        {
            return distribution(mMersenneTwisterGenerator);
        }

        std::mt19937 mMersenneTwisterGenerator;

        std::uniform_real_distribution<float> distribution;
    };

    
    class RNGIntRangeInAB
    {
        /** Generator for a Random Integer in the range [A, B]..
        \param[in] rangeA The lower value of the range.
        \param[in] rangeB The higher value of the range.
        */
        RNGIntRangeInAB(int rangeA, int rangeB)
        {
            std::random_device randomdevice;
            std::seed_seq seed{ randomdevice() };
            mMersenneTwisterGenerator = std::mt19937(seed);

            distribution = std::uniform_int_distribution<int>(rangeA, rangeB);

        };

        int generate()
        {
            return distribution(mMersenneTwisterGenerator);
        }

        std::mt19937 mMersenneTwisterGenerator;

        //	
        std::uniform_int_distribution<int> distribution;
    };

    //  SequenceMaker Template.
    template <typename T>
    class SequenceMaker
    {

    public:

        //	
        SequenceMaker() {}

        //	Get the current sequence index.
        uint32_t getSequenceIndex() const
        {
            return mSequenceIndex;
        }

        //	Get the next value in the sequence, incrementing by one.
        virtual T getNextSequenceValue()
        {
            //	All sequences must start with n = 0.
            assert(mSequenceIndex >= 0);

            //	Return the sequence value at the specified index.
            return getSequenceValue(mSequenceIndex);
        }

        //	Return the Sequence Value at the specified index.
        virtual T getSequenceValue(uint32_t sequenceIndex) const = 0;


    private:

        //	
        uint32_t mSequenceIndex = 0;
    };

    //	Generates a Van Der Corput Sequence.
    class VDCSequenceMaker : public SequenceMaker<float>
    {
    public:

        /**	Generate the VDC Sequence using the specified base at the provided index.
        \param[in] rangeA The lower value of the range.
        \param[in] rangeB The higher value of the range.
        */
        static float getVDCSequenceValue(uint32_t index, uint32_t base)
        {
            float x = 1.0f;


            float value = 0.0f;

            //	
            while (index > 0)
            {
                x = x / (float)base;

                value = value + x * (index % base);

                index = index / base;
            }

            //	
            return value;
        };


        //	VDC Sequence Constructor. The new base must be prime.
        VDCSequenceMaker(uint32_t newBase) : SequenceMaker(), mBase(newBase) {};

        //	Return the Sequence Value at the specified Index.
        float getSequenceValue(uint32_t sequenceIndex) const 
        {
            return getVDCSequenceValue(sequenceIndex, mBase);
        }

    protected:

        //
        uint32_t mBase;
    };

    //	Random Sequence Template.
    template <typename T>
    class RandomSequence
    {
    public:

        //	Return the Sequence.
        std::vector<T> & getSequence() const
        {
            return mSequence;
        };


    protected:

        //	Random Sequence.
        RandomSequence(uint32_t sequenceSize)
        {
            //	Cannot have a sequence of 0 size.
            assert(sequenceSize != 0);

            //	Allocate the entire sequence at once.
            mSequence = std::vector<T>(sequenceSize);

        };

        //	The Sequence.
        std::vector<T> mSequence;

    };


    //  NDimensional Random Sequence class.
    //  Uses a vector of floats underneath, with size corresponding to the Sequence Size * the Dimensions.
    template <typename T>
    class NDimensionalSequence : public RandomSequence<float>
    {
        public:

            NDimensionalSequence(uint32_t sequenceSize, uint32_t dimensionsCount) : public RandomSequence(sequenceSize * dimensionsCount), mDimensionsCount(dimensionsCount){};

            //  Return the Dimensions Count.
            virtual void getDimensionsCount() const  
            {
                return mDimensionsCount;
            };

            //  Return the Sequence Size.
            virtual void getSequenceSize() const
            {
                return getSequence() / mDimensionsCount;
            };

            //  Return the point in the array, as indexed for by the key, and returning the a pointer  start of the n-dimensional point in the array.
            virtual void getPointInArray(const T & key, float * & npoint) {} const = 0;
            
            //  The Dimensions Count.
            uint32_t mDimensionsCount;
    };



    //	Pure Random Sequence.
    class PureRandomSequence : public RandomSequence<float>
    {

    public:
        //	Pure Random Sequence from Mersenne Twister Engine.
        PureRandomSequence(uint32_t sequenceSize) : RandomSequence(sequenceSize)
        {
            PureRandomSequence(sequenceSize, 0.0, 1.0);
        }

        /** Generator for a Pure Random Sequence from Mersenne Twister Engine - [newLowerBound, newHigherBound).
        \param[in] rangeA The lower value of the range.
        \param[in] rangeB The higher value of the range.
        */
        PureRandomSequence(uint32_t sequenceSize, float newLowerBound, float newHigherBound) : RandomSequence(sequenceSize)
        {
            //	Check for valid bounds.
            assert(newLowerBound <= newHigherBound);

            //	
            mRNG = RNGFloatRangeInAExB(newLowerBound, newHigherBound);

            //	Set the Lower Bound.
            mLowerBound = newLowerBound;

            //	Set the Higher Bound.
            mHigherBound = newHigherBound;

            //	Generate the sequence.
            for (uint32_t i = 0; i < mSequence.size(); i++)
            {
                mSequence[i] = mRNG.generate();
            }
        }

    protected:

        RNGFloatRangeInAExB mRNG;

        //	
        float mLowerBound = 0.0;

        //
        float mHigherBound = 1.0;
    };

    //	Halton 2D Sequence.
    class HaltonSequence2D : public RandomSequence<glm::vec2>
    {

    public:

        /** Generator for a 2D Halton Sequence from the provided bases.
        \param[in] baseX The Base X to use.
        \param[in] baseY The Base Y to use.
        */
        HaltonSequence2D(uint32_t sequenceSize, uint32_t newBaseX, uint32_t newBaseY) : RandomSequence(sequenceSize), mBaseX(newBaseX), mBaseY(newBaseY)
        {
            //	
            for (uint32_t i = 0; i < mSequence.size(); i++)
            {
                float xValue = VDCSequenceMaker::getVDCSequenceValue(i, newBaseX);

                float yValue = VDCSequenceMaker::getVDCSequenceValue(i, newBaseY);

                mSequence[i] = glm::vec2(xValue, yValue);
            }
        }

    protected:
        //	
        uint32_t mBaseX = 0;

        //	
        uint32_t mBaseY = 0;
    };

    //	Hammersley 2D Sequence.
    class HammersleySequence2D : public RandomSequence<glm::vec2>
    {

    public:

        /** Generator for a 2D Hammersley Sequence from the provided bases.
        \param[in] newBase The Base to use.
        */
        HammersleySequence2D(uint32_t sequenceSize, uint32_t newBase) : RandomSequence(sequenceSize), mBase(newBase)
        {
            //	
            for (uint32_t i = 0; i < mSequence.size(); i++)
            {
                float xValue = ((float)i) / ((float)mSequence.size());

                float yValue = VDCSequenceMaker::getVDCSequenceValue(i, newBase);

                mSequence[i] = glm::vec2(xValue, yValue);
            }
        }
    protected:

        //	
        uint32_t mBase = 0;
    };

    //	Multi-Jittered 2D Sequence.
    class MultiJitteredSequence2D : public RandomSequence<glm::vec2>
    {

    public:

        /** Generator for a 2D Multi-Jittered Sequence for the unit square.
        \param[in] xPointsCount The number of points along the x axis.
        \param[in] yPointsCount The number of points along the y axis.
        */
        MultiJitteredSequence2D(uint32_t xPointsCount, uint32_t yPointsCount) : RandomSequence(xPointsCount * yPointsCount), mXPointsCount(xPointsCount), mYPointsCount(yPointsCount)
        {
            //	
            if (mXPointsCount * mYPointsCount == 1)
            {
                mSequence[0] = glm::vec2(0.5, 0.5);
                return;
            }

            //  Create the Canonical Arrangement.
            for (uint32_t col = 0; col < mXPointsCount; col++)
            {
                for (uint32_t row = 0; row < mYPointsCount; row++)
                {
                    uint32_t gridAccess = col * mYPointsCount + row;
                    float colf = (float)(col);
                    float rowf = (float)(row);
                    mSequence[gridAccess].x = ((rowf + (colf + mRNG.generate()) / (float)mXPointsCount) / (float)mYPointsCount);
                    mSequence[gridAccess].y = ((colf + (rowf + mRNG.generate()) / (float)mYPointsCount) / (float)mXPointsCount);
                }
            }

            //  Shuffle the X Coordinates for each column.
            for (uint32_t col = 0; col < mXPointsCount; col++)
            {
                for (uint32_t row = 0; row < mYPointsCount; row++)
                {
                    //  Select the Column.
                    uint32_t k = col + (uint32_t)(mRNG.generate() * (mXPointsCount - col));

                    //  Swap the x coordinates.
                    std::swap(mSequence[col * mYPointsCount + row].x, mSequence[k * mYPointsCount + row].x);
                }
            }


            //  Shuffle the Y Coordinate for each row.
            for (uint32_t row = 0; row < mYPointsCount; row++)
            {
                for (uint32_t col = 0; col < mXPointsCount; col++)
                {
                    //  Select the Row.
                    uint32_t k = row + (uint32_t)(mRNG.generate() * (mYPointsCount - row));

                    //  Swap the y coordinates.
                    std::swap(mSequence[col * mYPointsCount + row].y, mSequence[col * mYPointsCount + k].y);
                }
            }

        }

    protected:

        //	
        uint32_t mXPointsCount;
        uint32_t mYPointsCount;

        //	
        RNGFloatRangeInAExB mRNG;
    };


    /** Generator for a 2D Correlated Multi-Jittered Sequence for the unit square.
    \param[in] xPointsCount The number of points along the x axis.
    \param[in] yPointsCount The number of points along the y axis.
    */
    class CorrelatedMultiJitteredSequence2D : public RandomSequence<glm::vec2>
    {

    public:
        //	
        CorrelatedMultiJitteredSequence2D(uint32_t xPointsCount, uint32_t yPointsCount) : RandomSequence(xPointsCount * yPointsCount), mXPointsCount(xPointsCount), mYPointsCount(yPointsCount)
        {
            //	
            if (mXPointsCount * mYPointsCount == 1)
            {
                mSequence[0] = glm::vec2(0.5, 0.5);
                return;
            }

            //  Create the Canonical Arrangement.
            for (uint32_t col = 0; col < mXPointsCount; col++)
            {
                for (uint32_t row = 0; row < mYPointsCount; row++)
                {
                    uint32_t gridAccess = col * mYPointsCount + row;
                    float colf = (float)(col);
                    float rowf = (float)(row);
                    mSequence[gridAccess].x = ((rowf + (colf + mRNG.generate()) / (float)mXPointsCount) / (float)mYPointsCount);
                    mSequence[gridAccess].y = ((colf + (rowf + mRNG.generate()) / (float)mYPointsCount) / (float)mXPointsCount);
                }
            }

            //  Shuffle the X Coordinates for each column.
            for (uint32_t col = 0; col < mXPointsCount; col++)
            {
                //  Select the Column.
                uint32_t k = col + (uint32_t)(mRNG.generate() * (mXPointsCount - col));

                for (uint32_t row = 0; row < mYPointsCount; row++)
                {
                    //  Swap the x coordinates.
                    std::swap(mSequence[col * mYPointsCount + row].x, mSequence[k * mYPointsCount + row].x);
                }
            }


            //  Shuffle the Y Coordinate for each row.
            for (uint32_t row = 0; row < mYPointsCount; row++)
            {
                //  Select the Row.
                uint32_t k = row + (uint32_t)(mRNG.generate() * (mYPointsCount - row));

                for (uint32_t col = 0; col < mXPointsCount; col++)
                {
                    //  Swap the y coordinates.
                    std::swap(mSequence[col * mYPointsCount + row].y, mSequence[col * mYPointsCount + k].y);
                }
            }
        }

    protected:

        //	
        uint32_t mXPointsCount;
        uint32_t mYPointsCount;

        //	
        RNGFloatRangeInAExB mRNG;
    };



    //  Map the point to the Uniform Hemisphere.
    inline static glm::vec3 mapToUniformHemisphereSample(glm::vec2 point)
    {
        float phi = point.y * 2.0f * (float)M_PI;

        float cosTheta = 1.0f - point.x;

        float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

        return glm::vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    }

    //  Map the point to the Cosine Weighted Hemisphere.
    inline static glm::vec3 mapToCosineWeightedHemisphereSample(glm::vec2 point)
    {
        float phi = point.y * 2.0f * (float)M_PI;

        float cosTheta = sqrt(1.0f - point.x);

        float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

        return glm::vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    }

};

