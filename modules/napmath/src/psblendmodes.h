#pragma once
/**
 * All Photoshop blend modes.
 * taken from http://www.deepskycolors.com/archivo/2010/04/21/formulas-for-Photoshop-blending-modes.html
 * Operations should be done on normalized values.
 * Some of these functions are just included for completeness (multiply anyone?)
 */

#include <algorithm>

namespace nap
{
	namespace math
	{

		/**
		 * Darken
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T darken(T target, T blend)
		{
			return std::min(target, blend);
		}

		/**
		 * Multiply
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T multiply(T target, T blend)
		{
			return target * blend;
		}

		/**
		 * Color Burn
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T colorBurn(T target, T blend)
		{
			return 1 - (1 - target) / blend;
		}

		/**
		 * Linear Burn
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T linearBurn(T target, T blend)
		{
			return target + blend - 1;
		}

		/**
		 * Lighten
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T lighten(T target, T blend)
		{
			return std::max(target, blend);
		}

		/**
		 * Screen
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T screen(T target, T blend)
		{
			return 1 - (1 - target) * (1 - blend);
		}

		/**
		 * Color Dodge
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T colorDodge(T target, T blend)
		{
			return target / (1 - blend);
		}

		/**
		 * Linear Dodge
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T linearDodge(T target, T blend)
		{
			return target + blend;
		}

		/**
		 * Overlay
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T overlay(T target, T blend)
		{
			if (target > 0.5)
				return (1 - (1 - 2 * (target - 0.5)) * (1 - blend));
			return ((2 * target) * blend);
		}

		/**
		 * Soft Light
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T softLight(T target, T blend)
		{
			if (blend > 0.5)
				return (1 - (1 - target) * (1 - (blend - 0.5)));
			return (blend <= 0.5) * (target * (blend + 0.5));
		}

		/**
		 * Hard Light
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T hardLight(T target, T blend)
		{
			if (blend > 0.5)
				return (1 - (1 - target) * (1 - 2 * (blend - 0.5)));
			return (blend <= 0.5) * (target * (2 * blend));
		}

		/**
		 * Vivid Light
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T vividLight(T target, T blend)
		{
			if (blend > 0.5)
				return (1 - (1 - target) / (2 * (blend - 0.5)));
			return (target / (1 - 2 * blend));
		}

		/**
		 * Linear Light
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T linearLight(T target, T blend)
		{
			if (blend > 0.5)
				return (target + 2 * (blend - 0.5));
			return (blend <= 0.5) * (target + 2 * blend - 1);
		}
		/**
		 * Pin Light
		 *
		 * Non-Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T pinLight(T target, T blend)
		{
			if (blend > 0.5)
				return (max(target, 2 * (blend - 0.5)));
			return (blend <= 0.5) * (min(target, 2 * blend));
		}

		/**
		 * Difference
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T difference(T target, T blend)
		{
			return target - blend;
		}

		/**
		 * Exclusion
		 *
		 * Commutative
		 *
		 * @param target Base value
		 * @param blend Value to combine with
		 * @return The result
		 */
		template <typename T>
		T exclusion(T target, T blend)
		{
			return 0.5 - 2 * (target - 0.5) * (blend - 0.5);
		}
	}


}